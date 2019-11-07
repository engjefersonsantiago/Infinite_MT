#include <random>

#ifndef __POLICY__
#define __POLICY__


#include "pkt_common.hpp"
#include "controller.hpp"
#include "lookup_table.hpp"
#include "stats.hpp"
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

        Policier(lookup_table_t& lookup_table,
                    stats_table_t& stats_table) :
                    lookup_table_{lookup_table},
                    stats_table_{stats_table}
        {}

        virtual FiveTuple select_replacement_victim() const =0;

    protected:
        lookup_table_t& lookup_table_;
        stats_table_t& stats_table_;


};

template< typename lookup_table_t, typename  stats_table_t>
class RandomPolicy final: public Policier<lookup_table_t,stats_table_t>
{

    public:
        using five_tuple_vector_t = std::vector<FiveTuple>;
        using policer_t =  Policier<lookup_table_t, stats_table_t>;

    private:
        size_t entry_index_to_remove;
        // Controller Five Tuple Vector
        five_tuple_vector_t reference_five_tuple_vector_;

        // Hide the random generation mechanism
        size_t random_number_generation() const {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, lookup_table_t::LOOKUP_MEM_SIZE);
            return dis(gen);
        }

    public:

        RandomPolicy(lookup_table_t& lookup_table,
                        stats_table_t& stats_table) :
                        policer_t(lookup_table, stats_table)
        {
            for(const auto& elem : lookup_table)
                reference_five_tuple_vector_.push_back(elem.first);
        }

        virtual FiveTuple select_replacement_victim() const override {
            return reference_five_tuple_vector_[random_number_generation()];
        }
};

template<typename lookup_table_t, typename  stats_table_t>
class LRUPolicy final: public Policier<lookup_table_t,stats_table_t>
{

    public:
        using policer_t =  Policier<lookup_table_t, stats_table_t>;

        LRUPolicy(lookup_table_t& lookup_table,
                        stats_table_t& stats_table) :
                        policer_t(lookup_table, stats_table)
        {}

        virtual FiveTuple select_replacement_victim() const override {
            // Get the least recebtly used entry.
            // TUple + value associe
            // 1. Get stats
            return this->stats_table_.back().first;
        }

};

// Opt Policy uses a ordered list of further seem flows
// These flows need to be evicted
template<typename lookup_table_t, typename  stats_table_t>
using OPTPolicy = LRUPolicy<lookup_table_t, stats_table_t>;

#endif
