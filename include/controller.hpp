
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

template<size_t Lookup_Size_L1, size_t Lookup_Size_L2, typename Lookup_Value, size_t Sleep_Time = 0, typename Policier_t>
class Controller{


    public:
    // Types
    using inter_thread_digest_cpu = ThreadCommunication<tuple_pkt_size_pair_t>;
    using tuple_value_pair_t = std::pair<FiveTuple,Lookup_Value>;
    using five_tuple_vector_t = std::vector<FiveTuple>;

    Controller(LookupTable<Lookup_Size_L1, Lookup_Value>& lookup_table_L1, LookupTable<Lookup_Size_L2, Lookup_Value>& lookup_table_L2,  Policier ): 
    lookup_table_L1_(lookup_table_L1) , lookup_table_L2_(lookup_table_L2_), Policier {}



    // Messages echanges entre L1 et controller

    bool remove_entry_L1_cache(){
        auto& [tuple,value] = entry_to_remove;
        // TODO: Pourquoi un return statement ici?
        return lookup_table_L1_.delete_key(tuple);    
    }

    bool add_entry_L1_cache(){
        auto& [tuple,value] = entry_to_add;
        return lookup_table_L1_.add_key(tuple, value);

    }

    void harvest_stat_L1_cache(){

    }

    void harvest_full_stats_L1_cache(){
        // See question raised for harvest stats L2 cache
        // Policy related method.

    }

        // From 
    void process_digest_from_L1_cache(inter_thread_digest_cpu& digest_pkt){
        // Get
        digest_pkt.pull_message(this->tuple_size_pair_);

        // Entry to add 

        // Interpret the message
        // Select the entry to add ?


        // Le controlleur ne devrait pas directement manipuler les entree dans la cache L1 et L2?
            // Dans quel cas est-ce vrai?

        // Polic

        // Read the l1 lookup table cache occupancy.
            // Is the cache full ?

            // If not.
                // Select the in reference lookup table the entry to add.m
                // Insert associated rule
            // Else
                // Apply replacement policy.
                    // Select an entry to evict (from L1)

        // 


        policy.update();

    }
    


    // Interface to/from Main Memory


    // Interface to/from L1
        // To
    bool remove_entry_L2_cache(){
        auto& [tuple,value] = entry_to_remove;
        return lookup_table_L2_.delete_key(tuple);    
    }

    bool add_entry_L2_cache(){
        auto& [tuple,value] = entry_to_add;
        return lookup_table_L2_.add_key(tuple, value);

    }


    void harvest_stats_L2_cache(){
        // Where are the L2 cache stored?
            // Cointainer
        // Which stats?
            // Entry to evict
        // When?
            // Upon a cache miss?

            // Controller pull before inserting a new value?


    }



    private:
        std::unordered_map<FiveTuple, Lookup_Value> full_lookup_table_;
        LookupTable<Lookup_Size, Lookup_Value>& lookup_table_L1_; 
        LookupTable<Lookup_Size, Lookup_Value>& lookup_table_L2;
        five_tuple_vector_t reference_five_tuple_vector;


        // Stats - Coin
        StatsContainer<Size, Stats_Value>& stats_table_L1_; 
        StatsContainer<Size, Stats_Value>& stats_table_L2;

        // Entry
        tuple_value_pair_t entry_to_add;
        tuple_value_pair_t entry_to_remove;

        // Policy 
        Policier<> l1_policy;
        Policier<> l2_policy; 
        
}
// Interface vers L1
// from
// to


// Interface vers Mem Principale 
//from 
// to

// Write key / delete key.


// Interface recupere statistic 
// from  L1, main


// Class lookup_table Template Typename 
// Mutex

#endif // __CONTROLLER__ 
