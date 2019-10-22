
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

template<size_t Lookup_Size, typename Lookup_Value, size_t Sleep_Time = 0>
class Controller{


    public:
    // Types
    using inter_thread_digest_cpu = ThreadCommunication<tuple_pkt_size_pair_t>;
    using tuple_value_pair_t = std::pair<FiveTuple,Lookup_Value>;


    // Constructor 
    Controller(LookupTable<Lookup_Size, Lookup_Value>& lookup_table_L1, LookupTable<Lookup_Size, Lookup_Value>& lookup_table_L2): 
    lookup_table_L1_(lookup_table_L1) , lookup_table_L2_(lookup_table_L2_) {}



    // Messages echanges entre L1 et controller
    
    // Interface to/from L1
        // To
    bool remove_entry_L1_cache(){
        auto& [tuple,value] = entry_to_remove;
        return lookup_table_L1_.delete_key(tuple);    
    }

    bool add_entry_L1_cache(){
        auto& [tuple,value] = entry_to_add;
        return lookup_table_L1_.add_key(tuple, value);

    }


    void harvest_stats_L1_cache(){

    }

        // From 
    void process_digest_from_L1_cache(inter_thread_digest_cpu& digest_pkt){
        // Get
        digest_pkt.pull_message(this->tuple_size_pair_);

        // Process - Update Policy TBD 
        policy.update();

    }
    



    // Interface to/from Main Memory


    private:
        std::unordered_map<FiveTuple, Lookup_Value> full_lookup_table_;
        LookupTable<Lookup_Size, Lookup_Value>& lookup_table_L1_; 
        LookupTable<Lookup_Size, Lookup_Value>& lookup_table_L2;
        tuple_value_pair_t entry_to_add;
        tuple_value_pair_t entry_to_remove;
        
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
