
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

template<typename Lookup_Table_L1 , typename Lookup_Table_L2, typename Policier_t>
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
                    Policier_t& policy,
                    std::size_t slowdown
                    ) :
                    full_lookup_table_(full_lookup_table),
                    lookup_table_L1_(lookup_table_L1),
                    lookup_table_L2_(lookup_table_L2),
                    l1_policy_(policy),
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
                                        Policy& policy)
        {
            auto [timeout, step] =  digest_pkt.pull_message(tuple_size_pair_, slowdown_factor_);
            auto [five_tuple, size] = tuple_size_pair_;

            // Exit in case of timeout
            if (timeout)
            { 
                return true; 
            }

            // L1 Table full ? Identify a victim for eviction
            if (lookup_table_L1_.is_full()){
                auto evicted_key = policy.select_replacement_victim();
                [[maybe_unused]] auto ctrl_signal_removal = remove_entry_cache(lookup_table, evicted_key);
            }

            Value_t value = 0;
            // Insert the new value.
            auto lookup_it = full_lookup_table_.find(five_tuple);
            if (lookup_it == full_lookup_table_.end())
            {
                // Inserts a new key/velue to the full lookup table
                // default val = 0
                full_lookup_table_.insert({ five_tuple, value });
                std::cout << "Learned " << five_tuple << '\n';
            } else
            {
                value = lookup_it->second;
                std::cout << "Inserted " << five_tuple << '\n';
            }
            add_entry_cache(lookup_table, five_tuple, value);
            return false;

        }

        void process_digest(inter_thread_digest_cpu& l1_digest_pkt,
                                inter_thread_digest_cpu& l2_digest_pkt)
        {

            while (true) 
            {
                // L1 digest processing
                // Returns in case of a timeout
                if (process_digest_from_cache(l1_digest_pkt, lookup_table_L1_, l1_policy_))
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
        Policier_t l1_policy_;

        const std::size_t slowdown_factor_;

};


#endif // __CONTROLLER__
