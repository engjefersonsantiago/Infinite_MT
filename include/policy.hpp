
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



template< typename lookup_table_t, typename  stats_table_t>
class Policier{

    public: 
    using cache_entry_t = std::pair<FiveTuple,lookup_table_t::table_value_t>;

    Policier( lookup_table_t& lookup_table,
    ,      stats_table_t& stats_table): lookup_table_{lookup_table},  stats_table_{stats_table}  {} 
  
    virtual const cache_entry_t& select_replacement_victim() =0; 
 
 
    private:
    lookup_table_t& lookup_table_;
    stats_table_t& stats_table_; 


};

template< typename lookup_table_t, typename  stats_table_t>
class RandomPolicy final: public Policier<lookup_table_t,stats_table_t>{

    private:
        size_t entry_index_to_remove;
        // Controller Five Tuple Vector
        five_tuple_vector_t& reference_five_tuple_vector_;

        // Hide the random generation mechanism
        size_t random_number_generation(){
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, lookup_table_t::table_size_t);
            return dis(gen);
        }

    public:

        RandomPolicy(lookup_table_t& lookup_table,
    stats_table_t& stats_table,five_tuple_vector_t& reference_five_tuple_vector): Policier(lookup_table,stats_table), reference_five_tuple_vector_{reference_five_tuple_vector}
    {}
        virtual const cache_entry_t& select_replacement_victim() override {
            return reference_five_tuple_vector_[random_number_generation()];
        }
};

template< typename lookup_table_t, typename  stats_table_t>
class LRUPolicy final: public Policier<lookup_table_t,stats_table_t>{

    public:
        virtual const cache_entry_t& select_replacement_victim() override {
            // Get the least recebtly used entry.
            // TUple + value associe
            stats_table_.back();
        }
};


#endif