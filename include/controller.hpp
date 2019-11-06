
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
        using inter_thread_digest_cpu = ThreadCommunication<tuple_pkt_size_pair_t>;
        using tuple_value_pair_t = std::pair<FiveTuple,typename Lookup_Table_L1::table_value_t>;
        using five_tuple_vector_t = std::vector<FiveTuple>;

        Controller(Lookup_Table_L1& lookup_table_L1,
                    Lookup_Table_L2& lookup_table_L2,
                    Policier_t& policy ) :
                    lookup_table_L1_(lookup_table_L1),
                    lookup_table_L2_(lookup_table_L2),
                    l1_policy(policy)
        {}


        bool remove_entry_L1_cache(){
            auto& [tuple,value] = entry_to_remove;
            return lookup_table_L1_.remove(tuple);
        }

        bool add_entry_L1_cache(){
            auto& [tuple,value] = entry_to_add;
            return lookup_table_L1_.insert(tuple, value);

        }



        void process_digest_from_L1_cache(inter_thread_digest_cpu& digest_pkt){
            //TODO: Pourquoi un this->?
            auto [boolean,step] =  digest_pkt.pull_message(this->tuple_size_pair_);
            auto [five_tuple, size] = tuple_size_pair_;

            // L1 Table full ? Identify a victim for eviction
            if (lookup_table_L1_.is_full()){
                auto  [evicted_key,evicted_val] = l1_policy.select_replacement_victim();
                auto ctrl_signal_removal = l1_policy.remove(evicted_key);
            }
            // Insert the new value.
            auto value = full_lookup_table_[five_tuple];
            l1_policy.insert(five_tuple,value);

        }


        // Interface to/from Main Memory
        bool remove_entry_L2_cache(){
            auto& [tuple,value] = entry_to_remove;
            return lookup_table_L2_.delete_key(tuple);
        }

        bool add_entry_L2_cache(){
            auto& [tuple,value] = entry_to_add;
            return lookup_table_L2_.add_key(tuple, value);

        }


    private:
        std::unordered_map<FiveTuple, typename Lookup_Table_L1::table_value_t> full_lookup_table_;
        Lookup_Table_L1& lookup_table_L1_;
        Lookup_Table_L2& lookup_table_L2_;
        five_tuple_vector_t reference_five_tuple_vector;

        // Message
        tuple_pkt_size_pair_t tuple_size_pair_;
        // Stats - Coin
        //StatsContainer<Size, Stats_Value>& stats_table_L1_;
        //StatsContainer<Size, Stats_Value>& stats_table_L2;

        // Entry
        tuple_value_pair_t entry_to_add;
        tuple_value_pair_t entry_to_remove;

        // Policy
        Policier_t l1_policy;

};


#endif // __CONTROLLER__
