
// STL libs
#include <cstdint>
#include <array>
#include <tuple>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

#ifndef __CONTROLLER__
#define __CONTROLLER__

#include "pkt_common.hpp"
#include "packet_processing.hpp"
#include "lookup_table.hpp"

template<typename Lookup_Table_L1 , typename Lookup_Table_L2, typename Policier1_t>
class Controller
{

    public:
        // Types
        using Value_t = typename Lookup_Table_L1::table_value_t;
        using tuple_value_pair_t = std::pair<FiveTuple, Value_t>;
        using five_tuple_vector_t = std::vector<FiveTuple>;
        using full_look_table_t = std::unordered_map<FiveTuple, Value_t>;

        // CTOR
        Controller(full_look_table_t& full_lookup_table,
                    Lookup_Table_L1& lookup_table_L1,
                    Lookup_Table_L2& lookup_table_L2,
                    Policier1_t& policy1,
                    const std::size_t slowdown
                    ) :
                    full_lookup_table_(full_lookup_table),
                    lookup_table_L1_(lookup_table_L1),
                    lookup_table_L2_(lookup_table_L2),
                    l1_policy_(policy1),
                    slowdown_factor_(slowdown)
        {}


        // Add/remove entry uses the same call
        template<typename Lookup>
        auto remove_entry_cache(Lookup& lookup_table, FiveTuple tuple)
        {
            return lookup_table.remove(tuple);
        }

        template<typename Lookup>
        auto add_entry_cache(Lookup& lookup_table, FiveTuple tuple, Value_t value)
        {
            return lookup_table.insert(tuple, value);
        }

        template<typename Lookup, typename Policy>
        auto process_digest_from_cache(inter_thread_digest_cpu& digest_pkt,
                                        Lookup& lookup_table,
                                        Policy& policy,
                                        std::size_t& punted_pkts)
        {

            auto [timeout, step] =  digest_pkt.pull_message(tuple_size_pair_, slowdown_factor_);
            auto [five_tuple, size_or_timestamp] = tuple_size_pair_;
            ++punted_pkts;

            //std::cout << "------------- " << timeout << ", " << step << " ------------------\n";
            // Exit in case of timeout
            if (timeout)
            { 
                return true; 
            }

            // Lookup Table full ? Identify a victim for eviction
            if (lookup_table_L1_.is_full()/* && (lookup_table_L1_.find(five_tuple) == lookup_table_L1_.end())*/)
            {
            
                // Select candidate
                five_tuple = l1_policy_.select_promotion_candidate();
                if (lookup_table_L1_.find(five_tuple) == lookup_table_L1_.end())
                {
                    auto evicted_key = l1_policy_.select_replacement_victim(five_tuple, size_or_timestamp);
                    auto ctrl_signal_removal = remove_entry_cache(lookup_table_L1_, evicted_key);
                    debug(std::cout << "Remove function: " << ctrl_signal_removal << '\n';)
                    if (ctrl_signal_removal)
                    {
                        debug(std::cout << "-----------------------------------------------------------\n";)
                        debug(std::cout << "Removing: " << evicted_key << "Current occupancy: " << lookup_table_L1_.occupancy() << '\n';)
                        debug(std::cout << "-----------------------------------------------------------\n";)
                    } else
                    {
                        std::cout << "-----------------------------------------------------------\n";
                        std::cout << "Replacement l1_policy_ failed." << evicted_key << " not present\n";
                        std::cout << "-----------------------------------------------------------\n";
                        //int i;
                        //std::cin >> i;
                    }
                } else {
                    std::cout << "-----------------------------------------------------------\n";
                    std::cout << "Entry already in the lookup table." << five_tuple << '\n';
                    std::cout << "-----------------------------------------------------------\n";
                }
            } else {
                //
                auto tuple_compare = [=](const auto& elem) {
                    return elem.first == five_tuple;
                };

                auto found = l1_policy_.stats_table().get_stats().find_if(tuple_compare);
                if (found ==  l1_policy_.stats_table().get_stats().end())
                {
                    l1_policy_.stats_table().get_stats().insert(std::make_pair(five_tuple, size_or_timestamp), [](){}, [](){}); 
                }
            }

            Value_t value = 0;
            // Insert the new value.
#if 0
            auto lookup_it = full_lookup_table_.find(five_tuple);
            if (lookup_it == full_lookup_table_.end())
            {
                // Inserts a new key/velue to the full lookup table
                // default val = 0
                full_lookup_table_.insert({ five_tuple, value });
                debug(std::cout << "Learned " << five_tuple << '\n';)
            } else
            {
                value = lookup_it->second;
                //std::cout << "Inserted " << five_tuple << '\n';
            }
#endif
            if (!add_entry_cache(lookup_table_L1_, five_tuple, value))
            {
                std::cout << "--------------------------\n";
                std::cout << "Insertion failed\n";
                std::cout << "--------------------------\n";
            }
            debug(std::cout << "Digested " << punted_pkts << " packets, Inserting: " << five_tuple << ", Step: " << step << ", Current occupancy: " << lookup_table_L1_.occupancy() << '\n';)
            return false;
        }

        void process_digest(const bool run_forever, inter_thread_digest_cpu& l1_digest_pkt,
                                inter_thread_digest_cpu& l2_digest_pkt)
        {

            while (true) 
            {
                // L1 digest processing
                // Returns in case of a timeout
                if (process_digest_from_cache(l1_digest_pkt, lookup_table_L1_, l1_policy_, l1_punted_pkts) || !run_forever)
                {
                    break;
                }

                // TODO
                // L2 digest processing
            }
        }


    private:
        full_look_table_t full_lookup_table_;
        Lookup_Table_L1& lookup_table_L1_;
        Lookup_Table_L2& lookup_table_L2_;
        five_tuple_vector_t reference_five_tuple_vector;

        // Message
        tuple_pkt_size_pair_t tuple_size_pair_;
        // Stats - Coin
        //StatsContainer<Size, Stats_Value>& stats_table_L1_;
        //StatsContainer<Size, Stats_Value>& stats_table_L2;

        // Policy
        Policier1_t l1_policy_;

        std::size_t l1_punted_pkts = 0;

        const std::size_t slowdown_factor_;

};


#endif // __CONTROLLER__
