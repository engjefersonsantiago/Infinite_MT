
#ifndef __POLICY__
#define __POLICY__


#include "pkt_common.hpp"
#include "lookup_table.hpp"

// Quels sont les composants communs a une politique de cache.

// Base sur un input (Cache Stats) + Event (Cache Miss), identifie l'entree a ajouter
// (lookup container controller) et l entree a retirer.
// Interface avec le controller. 
// Policy doit pouvoir s

// Policier is specialized for a given politic. 
// Currently assume that L1 cache enries are updated following a cache miss.
// No stats 


// Policier (Cache Management Unit)
template< size_t Lookup_Size, typename Lookup_Value,typename Stats_Value>
class Policier{

    public: 
    using cache_entry_t = std::pair<FiveTuple,Lookup_Value>;
    using inter_thread_cpu_to_cache = ThreadCommunication<FiveTuple,Lookup_Value>;


    Policier(inter_thread_digest_cpu& l1_to_cpu_comm, inter_thread_digest_cpu& l2_to_cpu_comm,
    inter_thread_cpu_to_cache& cpu_to_l1_comm, inter_thread_cpu_to_cache& cpu_to_l2_comm ,
    LookupTable<Lookup_Size, Lookup_Value>& lookup_table): l1_to_cpu_comm_{l1_to_cpu_comm}, l2_to_cpu_comm_{l2_to_cpu_comm},
     cpu_to_l1_comm_{cpu_to_l1_comm}, cpu_to_l2_comm{cpu_to_l2_comm}, lookup_table_{lookup_table} {} 
     
    // Entry Management - Could also be entries.
    virtual void promote_entry_to_L1_cache(){

    }
    virtual void remove_entry_from_L1_cache(){

    };

    virtual void promote_entry_to_L2_cache() 
    }

    virtual void remove_entry_from_L2_cache()

    virtual void process_pkt_digest(){

    }

    
    // Harvest Stats
    // TODO: When stats should be harvestest?
    virtual void get_L1_cache_stats(){

    }

    virtual void get_L2_cache_stats(){

    }




    private:
    // Lookup Table 
    LookupTable<Lookup_Size, Lookup_Value>& lookup_table_;

    // Pkt Digest
    tuple_pkt_size_pair_t tuple_size_pair_;

    // Stats
    // Container? 

    // Entry(ies) to remove / insert
    cache_entry_t entry_to_remove_;
    cache_entry_t entry_to_promote_;

    // Inter-thread communication to CPU
    inter_thread_digest_cpu& l1_to_cpu_comm_;
    inter_thread_digest_cpu& l2_to_cpu_comm_;

    // Inter-thread communication from CPU
    inter_thread_cpu_to_cache& cpu_to_l1_comm_;
    inter_thread_cpu_to_cache& cpu_to_l2_comm_;


};

#endif