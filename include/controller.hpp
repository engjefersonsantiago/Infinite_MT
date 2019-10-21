
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


template<size_t Lookup_Size, typename Lookup_Value, size_t Sleep_Time = 0>
class Controller{


    public:

    // Types
    using inter_thread_digest_cpu = ThreadCommunication<tuple_pkt_size_pair_t>;
    using inter_thread_write_from_controller_t = ThreadCommunication< >;



    // Constructor 
    Controller(): {}
    // Messages echanges entre L1 et controller


    // Interface to/from L1
        // To
    void remove_entry_L1_cache(){


    }

    void add_entry_L1_cache(){
        // Qui donne la reference vers la lookup table

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


    protected:
    std::unordered_map<FiveTuple, Lookup_Value> full_lookup_table_;
    std::unordered_map<FiveTuple, Lookup_Value> lookup_table_L1_; // Which keys are held in the L1 cache?

    inter_thread_comm_t write_to_l1_;
    inter_thread_comm_t read_stats_from_l1_;
    inter_thread_comm_t write_to_main_mem_;
    inter_thread_comm_t read_stats_from_main_mem_;
    tuple_pkt_size_pair_t tuple_size_pair_;
    tuple_pkt_size_pair_t entry_to_add_;


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
