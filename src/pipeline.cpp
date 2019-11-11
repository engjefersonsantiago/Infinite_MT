#include<thread>
#include<iostream>
#include<chrono>

#define DEBUG

#include "pkt_common.hpp"
#include "parse_pcap.hpp"
#include "packet_processing.hpp"
#include "policy.hpp"

using cache_stats_t = LRUCacheStats<32, std::size_t>;
using base_l1_pkt_process_t = PacketProcessing<2, std::size_t, cache_stats_t>;
using cache_l1_t = CacheL1PacketProcessing<2, std::size_t, cache_stats_t>;
using base_l2_pkt_process_t = PacketProcessing<65536, std::size_t, cache_stats_t>;
using cache_l2_t = CacheL2PacketProcessing<65536, std::size_t, cache_stats_t>;
using LRU_policy_t = LRUPolicy<cache_l1_t::lookup_table_t, cache_stats_t >;
using Random_policy_t = RandomPolicy<cache_l1_t::lookup_table_t, cache_stats_t >;
using controller_t = Controller<typename cache_l1_t::lookup_table_t, typename cache_l2_t::lookup_table_t, Random_policy_t>;

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

    // Init lookup table
    auto unique_tuples = filter_unique_tuples_from_trace(pcap_file);
    std::cout << "Identified " << unique_tuples.size() << " unique tuples\n";

    // Populating lookup tables
#if 0
    for (const auto& tuple : unique_tuples) {
        if (!base_cache_l1.lookup_table().is_full()) {
            base_cache_l1.lookup_table().data().insert({ tuple, 0 });
        }
        if (!base_cache_l2.lookup_table().is_full()) {
            base_cache_l2.lookup_table().data().insert({ tuple, 0 });
        }
    }
#endif

    // Inter thread communication
    inter_thread_comm_t parse_to_l1_comm;
    inter_thread_comm_t l1_to_l2_comm(10);
    inter_thread_comm_t l2_to_dummy_comm;
    inter_thread_digest_cpu l1_to_cpu_comm;
    inter_thread_digest_cpu l2_to_cpu_comm;

    // Parser
    ParsePackets parse_pkts(pcap_file, timestamp_file);

    // Cache L1
    cache_l1_t cache_l1 (parse_to_l1_comm, l1_to_l2_comm, l1_to_cpu_comm);
    base_l1_pkt_process_t& base_cache_l1 = cache_l1;

    // Cache L2
    cache_l2_t cache_l2 (l1_to_l2_comm, l2_to_dummy_comm, l2_to_cpu_comm);
    base_l2_pkt_process_t& base_cache_l2 = cache_l2;

    // Policy
    LRU_policy_t lru_policy(base_cache_l1.lookup_table(),base_cache_l1.stats_table());
    Random_policy_t random_policy(base_cache_l1.lookup_table(),base_cache_l1.stats_table());

    // Controller with LRU Policy
    controller_t controller(base_cache_l2.lookup_table().data(),
                            base_cache_l1.lookup_table(),
                            base_cache_l2.lookup_table(),
                            random_policy,
                            CACHE_HOST_PROC_SLOWDOWN_FACTOR);

    auto start = std::chrono::system_clock::now();


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
    std::thread thread_controller(&controller_t::process_digest,
                                    controller,
                                    std::ref(l1_to_cpu_comm),
                                    std::ref(l2_to_cpu_comm)
                                );

    // TODO: Policer and kernel that ensure that all tasks are completed until the next time slot is started.

    thread_parse_pkt.join();
    std::cout << "Parser joined\n";
    thread_cache_l1.join();
    std::cout << "L1 joined\n";
    thread_cache_l2.join();
    std::cout << "L2 joined\n";
    thread_controller.join();
    std::cout << "Controller joined\n";
   
    controller.process_digest(l1_to_cpu_comm, l2_to_cpu_comm); 
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";

}
