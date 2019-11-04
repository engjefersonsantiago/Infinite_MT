#include<thread>
#include<iostream>
#include<chrono>

#include "pkt_common.hpp"
#include "parse_pcap.hpp"
#include "packet_processing.hpp"

using cache_stats_t = LRUCacheStats<32, std::size_t>;
using base_l1_pkt_process_t = PacketProcessing<1024, std::size_t, cache_stats_t>;
using cache_l1_t = CacheL1PacketProcessing<1024, std::size_t, cache_stats_t>;
using base_l2_pkt_process_t = PacketProcessing<65536, std::size_t, cache_stats_t>;
using cache_l2_t = CacheL2PacketProcessing<65536, std::size_t, cache_stats_t>;

//TODO: Add Policer.


static constexpr auto CACHE_L1_TYPE = CacheType::LRU;
static constexpr auto CACHE_L2_TYPE = CacheType::LRU;

int main() {

    std::cout << "Enter the PCAP file name\n";
    std::string pcap_file;
    std::cin >> pcap_file;
    std::cout << "Enter the timestamp file name\n";
    std::string timestamp_file;
    std::cin >> timestamp_file;
    std::cout << "Enter the amount of sleep time in ns for the parser thread\n";
    std::size_t sleep_time;
    std::cin >> sleep_time;

    // Inter thread communication
    inter_thread_comm_t parse_to_l1_comm;
    inter_thread_comm_t l1_to_l2_comm(10);
    inter_thread_comm_t l2_to_dummy_comm;
    inter_thread_digest_cpu l1_to_cpu_comm;
    inter_thread_digest_cpu l2_to_cpu_comm;
    //TODO Add communication CPU to cache

    // Parser
    ParsePackets parse_pkts(pcap_file, timestamp_file);

    // Cache L1
    cache_l1_t cache_l1 (parse_to_l1_comm, l1_to_l2_comm, l1_to_cpu_comm);
    base_l1_pkt_process_t& base_cache_l1 = cache_l1;

    // Cache L2
    cache_l2_t cache_l2 (l1_to_l2_comm, l2_to_dummy_comm, l2_to_cpu_comm);
    base_l2_pkt_process_t& base_cache_l2 = cache_l2;

    auto start = std::chrono::system_clock::now();

    // Init lookup table
    auto unique_tuples = filter_unique_tuples_from_trace(pcap_file);
    std::cout << "Identified " << unique_tuples.size() << " unique tuples\n";

    // Populating lookup tables
    for (const auto& tuple : unique_tuples) {
        if (!base_cache_l1.lookup_table().full()) {
            base_cache_l1.lookup_table().data().insert({ tuple, 0 });
        }
        if (!base_cache_l2.lookup_table().full()) {
            base_cache_l2.lookup_table().data().insert({ tuple, 0 });
        }
    }

    // Start processing threads
    std::thread thread_parse_pkt(&ParsePackets::from_pcap_file,
                                    parse_pkts,
                                    std::ref(parse_to_l1_comm),
                                    nano_second_t(sleep_time)
                                );
    std::thread thread_cache_l1(&base_l1_pkt_process_t::process_packet,
                                    std::ref(base_cache_l1),
                                    CACHE_L1_PROC_SLOWDOWN_FACTOR,
                                    CACHE_L1_TYPE
                                );
    std::thread thread_cache_l2(&base_l2_pkt_process_t::process_packet,
                                    std::ref(base_cache_l2),
                                    CACHE_L2_PROC_SLOWDOWN_FACTOR,
                                    CACHE_L2_TYPE
                                );


    // TODO: Policer and kernel that ensure that all tasks are completed until the next time slot is started.

    thread_parse_pkt.join();
    std::cout << "Parser joined\n";
    thread_cache_l1.join();
    std::cout << "L1 joined\n";
    thread_cache_l2.join();
    std::cout << "L2 joined\n";

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";

}
