
#ifndef __POLICY__
#define __POLICY__


#include "pkt_common.hpp"
#include "lookup_table.hpp"
#include "stats.hpp"
#include <random>
// Quels sont les composants communs a une politique de cache.

// Base sur un input (Cache Stats) + Event (Cache Miss), identifie l'entree a ajouter
// (lookup container controller) et l entree a retirer.
// Interface avec le controller. 
// Policy doit pouvoir s

// Policier is specialized for a given politic. 
// Currently assume that L1 cache enries are updated following a cache miss.



// Policier (Cache Management Unit)
template< size_t Lookup_Size, typename Lookup_Value, size_t Stats_Size, typename Stats_Value>
class Policier{

    public: 
    using cache_entry_t = std::pair<FiveTuple,Lookup_Value>;

    Policier( LookupTable<Lookup_Size, Lookup_Value>& lookup_table,
    ,      StatsContainer<Size, Stats_Value>& stats_table, five_tuple_vector_t& reference_five_tuple_vector): lookup_table_{lookup_table},  stats_table_{stats_table},
     {} 
     

  
    virtual cache_entry_t select_replacement_victim() =0; 
 
 
    private:
    // Lookup Table 
    LookupTable<Lookup_Size, Lookup_Value>& lookup_table_;

    // Controller Five Tuple Vector
    five_tuple_vector_t& reference_five_tuple_vector_;

    // Pkt Digest
    tuple_pkt_size_pair_t tuple_size_pair_;

    // Stats
    CacheStats<Stats_Size, Stats_Value>& stats_table_; 

    // Policy Type - Info Purpose
    CacheType cache_type;
};

template< size_t Lookup_Size, typename Lookup_Value, size_t Stats_Size, typename Stats_Value>
class RandomPolicy final: public Policier<Lookup_Size,Lookup_Value,Stats_Size,Stats_Value>

    private:
        size_t entry_index_to_remove;
        constexpr auto Table_Size = Stats_Size;

        // Hide the random generation mechanism
        size_t random_number_generation(){
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, Table_Size);
            return dis(gen);
        }

    public:
        virtual cache_entry_t select_replacement_victim(){
            return reference_five_tuple_vector_[random_number_generation()];
        }



#endif