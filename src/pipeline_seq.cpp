#include<thread>
#include<iostream>
#include<chrono>
#include<typeinfo>

#include "pkt_common.hpp"
#include "parse_pcap.hpp"
#include "packet_processing.hpp"
#include "policy.hpp"
#include "pipeline_params.hpp"


static constexpr auto CACHE_L1_TYPE = (L1_CACHE_POLICY == CacheType::OPT) ? CacheType::OPT : ((L1_CACHE_POLICY == CacheType::LFU) ? CacheType::LFU : CacheType::LRU);
static constexpr auto CACHE_L2_TYPE = (L2_CACHE_POLICY == CacheType::OPT) ? CacheType::OPT : ((L2_CACHE_POLICY == CacheType::LFU) ? CacheType::LFU : CacheType::LRU);

// Cache stats for each cache level
// LFU and LFU modif have the same stats
using cache_stats_t = std::conditional_t<L1_CACHE_POLICY == CacheType::LFU || L1_CACHE_POLICY == CacheType::LFU_MODIF, LFUCacheStats<L1_CACHE_STATS_SIZE, std::size_t>, LRUCacheStats<L1_CACHE_STATS_SIZE, std::size_t>>;

using cache_l2_stats_t = MFUCacheStats<L2_CACHE_STATS_SIZE, std::size_t>;

using base_l1_pkt_process_t = PacketProcessing<L1_CACHE_SIZE, std::size_t, cache_stats_t>;
using cache_l1_t = CacheL1PacketProcessing<L1_CACHE_SIZE, std::size_t, cache_stats_t>;

using base_l2_pkt_process_t = PacketProcessing<L2_CACHE_SIZE, std::size_t, cache_l2_stats_t>;
using cache_l2_t = CacheL2PacketProcessing<L2_CACHE_SIZE, std::size_t, cache_l2_stats_t>;

// Create duplicates for each policy: promotion and eviction
using LRU_policy_t = LRUPolicy<cache_l1_t::lookup_table_t, cache_stats_t >;
using LFU_policy_t = LFUPolicy<cache_l1_t::lookup_table_t, cache_stats_t >;
using LFU_Modif_policy_t = LFUModifPolicy<cache_l1_t::lookup_table_t, cache_stats_t >;
using Random_policy_t = RandomPolicy<cache_l1_t::lookup_table_t, cache_stats_t >;

using OPT_policy_t = OPTPolicy<cache_l1_t::lookup_table_t, cache_stats_t>;
// TODO:
// Specialize controller with two policies: Evition and Promotion
using Policy = std::conditional_t<L1_CACHE_POLICY == CacheType::LFU, LFU_policy_t,
                                    std::conditional_t<L1_CACHE_POLICY == CacheType::LFU_MODIF, LFU_Modif_policy_t,
                                    std::conditional_t<L1_CACHE_POLICY == CacheType::OPT, OPT_policy_t,
                                    std::conditional_t<L1_CACHE_POLICY == CacheType::LRU, LRU_policy_t, Random_policy_t>>>>;

using Promo_Policy = MFUPolicy<cache_l2_t::lookup_table_t, cache_l2_stats_t>;

using controller_t = Controller<typename cache_l1_t::lookup_table_t, typename cache_l2_t::lookup_table_t, Policy, Promo_Policy>;

//TODO: Add Policer.

int main(int argc, char** argv)
{

    if (argc != 3) {
    // First parameter: PCAP
    // Second parameter: Times
        std::cout << "Wrong number of paramenters\n";
        std::cout << "Usage: ./pipeline <pcap_file> <timestamp_file>\n";
        return -1;
    } else {
        std::cout << "Runnig pipeline application with parameters: " << argv[1] << " and " << argv[2] << '\n';
    }
    const std::string pcap_file { argv[1] };
    const std::string timestamp_file { argv[2] };

    // Inter thread communication
    inter_thread_comm_t parse_to_l1_comm;
    inter_thread_comm_t l1_to_l2_comm;
    inter_thread_comm_t l2_to_dummy_comm;
    inter_thread_digest_cpu l1_to_cpu_comm(1000);
    inter_thread_digest_cpu l2_to_cpu_comm;

    // Parser
    ParsePackets parse_pkts(pcap_file, timestamp_file);

    // Cache L1
    cache_l1_t cache_l1 (parse_to_l1_comm, l1_to_l2_comm, l1_to_cpu_comm);
    base_l1_pkt_process_t& base_cache_l1 = cache_l1;

    // Cache L2
    cache_l2_t cache_l2 (l1_to_l2_comm, l2_to_dummy_comm, l2_to_cpu_comm);
    base_l2_pkt_process_t& base_cache_l2 = cache_l2;

    // Init lookup table
    if constexpr (L1_CACHE_INIT_STS == CacheInit::FULL || L2_CACHE_INIT_STS == CacheInit::FULL)
    {
        auto unique_tuples = filter_unique_tuples_from_trace(pcap_file);
        std::cout << "Identified " << unique_tuples.size() << " unique tuples\n";
        // Populating lookup tables
        for (const auto& tuple : unique_tuples) {
            if (!base_cache_l1.lookup_table().is_full() && L1_CACHE_INIT_STS == CacheInit::FULL) {
                base_cache_l1.lookup_table().data().insert(std::make_pair(tuple, 0ul));
                base_cache_l1.stats_table().get_stats().insert(std::make_pair(tuple, 0ul), [](){},[](){});
            }
            if (!base_cache_l2.lookup_table().is_full() && L2_CACHE_INIT_STS == CacheInit::FULL) {
                base_cache_l2.lookup_table().data().insert(std::make_pair(tuple, 0ul));
                base_cache_l2.stats_table().get_stats().insert(std::make_pair(tuple, 0ul), [](){},[](){});
            }
        }
    }

    // Policy
    LRU_policy_t lru_policy(base_cache_l1.lookup_table(),base_cache_l1.stats_table());
    LFU_policy_t lfu_policy(base_cache_l1.lookup_table(),base_cache_l1.stats_table(), LFU_COUNTER_TYPE);
    LFU_Modif_policy_t lfu_modif_policy(base_cache_l1.lookup_table(),base_cache_l1.stats_table(), LFU_COUNTER_TYPE);
    MFUPolicy mfu_policy(base_cache_l2.lookup_table(),base_cache_l2.stats_table(), LFU_COUNTER_TYPE); // MPU for promotion
    Random_policy_t random_policy(base_cache_l1.lookup_table(),base_cache_l1.stats_table());
    OPT_policy_t opt_policy(base_cache_l1.lookup_table(),base_cache_l1.stats_table(),pcap_file);
    if constexpr (L1_CACHE_POLICY == CacheType::OPT)
        opt_policy.build_five_tuple_history();
    //

    std::tuple<LRU_policy_t, LFU_policy_t, LFU_Modif_policy_t, Random_policy_t, OPT_policy_t> policy { lru_policy, lfu_policy, lfu_modif_policy, random_policy , opt_policy };

    // Controller with LRU Policy
    controller_t controller(base_cache_l2.lookup_table().data(),
                            base_cache_l1.lookup_table(),
                            base_cache_l2.lookup_table(),
                            std::get<Policy>(policy),
                            mfu_policy,
                            CACHE_HOST_PROC_SLOWDOWN_FACTOR);

    auto start = std::chrono::system_clock::now();

    std::cout << "=======================================\n";
    // Start processing threads
    while (parse_pkts.from_pcap_file(false, parse_to_l1_comm))
    {
        base_cache_l1.process_packet(false, CACHE_L1_PROC_SLOWDOWN_FACTOR, CACHE_L1_TYPE, LFU_COUNTER_TYPE);
        base_cache_l2.process_packet(false, CACHE_L2_PROC_SLOWDOWN_FACTOR, CACHE_L2_TYPE, LFU_COUNTER_TYPE);
        controller.process_digest(false, l1_to_cpu_comm,l2_to_cpu_comm);
    }

    base_cache_l1.print_status();

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";

}
