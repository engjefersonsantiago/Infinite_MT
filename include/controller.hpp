
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


template<typename Lookup_Value>
class Controller{


    public:

    // Types
    using inter_thread_read_stats_t = ThreadCommunication< >;
    using inter_thread_write_from_controller_t = ThreadCommunication< >;

    // Messages echanges entre L1 et controller


    // Interface to/from L1
        // To
    remove_entry_L1_cache(){
        

    }

    add_entry_L1_cache(){

    }


    harvest_stats_L1_cache(){

    }

        // From 
    process_digest_from_L1_cache(){

        // Synchronisation

    }
    



    // Interface to/from Main Memory


    protected:
    std::unordered_map<FiveTuple, Lookup_Value> lookup_table_;
    inter_thread_comm_t write_to_l1_;
    inter_thread_comm_t read_stats_from_l1_;
    inter_thread_comm_t write_to_main_mem_;
    inter_thread_comm_t read_stats_from_main_mem_;

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
