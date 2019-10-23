#include<thread>
#include<iostream>
#include<chrono>

#include "pkt_common.hpp"
#include "parse_pcap.hpp"
#include "packet_processing.hpp"

using base_l1_pkt_process_t = PacketProcessing<1024, int, int>;
using cache_l1_t = CacheL1PacketProcessing<1024, int, int>;
using base_l2_pkt_process_t = PacketProcessing<65536, int, int>;
using cache_l2_t = CacheL2PacketProcessing<65536, int, int>;

int main() {

    std::cout << "Enter the PCAP file name\n";
    std::string pcap_file;
    std::cin >> pcap_file;
    std::cout << "Enter the timestamp file name\n";
    std::string timestamp_file;
    std::cin >> timestamp_file;
    std::cout << "Enter the amount of sleep time in ns for the parser thread\n";
    size_t sleep_time;
    std::cin >> sleep_time;

    // Inter thread communication
    inter_thread_comm_t parse_to_l1_comm;
    inter_thread_comm_t l1_to_l2_comm;
    inter_thread_comm_t l2_to_dummy_comm;

    // Parser
    ParsePackets parse_pkts(pcap_file, timestamp_file);

    // Cache L1
    cache_l1_t cache_l1;
    base_l1_pkt_process_t& base_cache_l1 = cache_l1;

    // Cache L1
    cache_l2_t cache_l2;
    base_l2_pkt_process_t& base_cache_l2 = cache_l2;

    auto start = std::chrono::system_clock::now();

    std::thread thread_parse_pkt(&ParsePackets::from_pcap_file, parse_pkts, std::ref(parse_to_l1_comm), nano_second_t(sleep_time));
    std::thread thread_cache_l1(&base_l1_pkt_process_t::process_packet, std::ref(base_cache_l1), std::ref(parse_to_l1_comm), std::ref(l1_to_l2_comm));
    std::thread thread_cache_l2(&base_l2_pkt_process_t::process_packet, std::ref(base_cache_l2), std::ref(l1_to_l2_comm), std::ref(l2_to_dummy_comm));

    thread_parse_pkt.join();
    std::cout << "Parser joined\n";
    thread_cache_l1.join();
    std::cout << "L1 joined\n";

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";

}
