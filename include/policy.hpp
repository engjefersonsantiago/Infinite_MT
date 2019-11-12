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

template<typename lookup_table_t, typename  stats_table_t>
class Policier{

    public:

        Policier(const lookup_table_t& lookup_table,
                    const stats_table_t& stats_table) :
                    lookup_table_{lookup_table},
                    stats_table_{stats_table}
        {}

        virtual FiveTuple select_replacement_victim() const =0;

    protected:
        const lookup_table_t& lookup_table_;
        const stats_table_t& stats_table_;

};

template<typename lookup_table_t, typename  stats_table_t>
class RandomPolicy final: public Policier<lookup_table_t,stats_table_t>
{

    public:
        using policer_t = Policier<lookup_table_t, stats_table_t>;

    private:

        // Hide the random generation mechanism
        std::size_t random_number_generation() const
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, lookup_table_t::LOOKUP_MEM_SIZE);
            return dis(gen);
        }

    public:

        RandomPolicy(const lookup_table_t& lookup_table,
                        const stats_table_t& stats_table) :
                        policer_t(lookup_table, stats_table)
        {}

        virtual FiveTuple select_replacement_victim() const override
        {
            // From https://stackoverflow.com/questions/27024269/select-random-element-in-an-unordered-map
            const auto tuple = this->lookup_table_.indexed_iter(random_number_generation()).first; 
            debug(std::cout << "Selected " << tuple << " for replacement\n";)
            return tuple;
        }
};

template<typename lookup_table_t, typename  stats_table_t>
class LRUPolicy final: public Policier<lookup_table_t,stats_table_t>
{

    public:
        using policer_t =  Policier<lookup_table_t, stats_table_t>;

        LRUPolicy(const lookup_table_t& lookup_table,
                        const stats_table_t& stats_table) :
                        policer_t(lookup_table, stats_table)
        {}

        virtual FiveTuple select_replacement_victim() const override
        {
            // Get the least recently used entry.
            // TUple + value associe
            // 1. Get stats
            return this->stats_table_.back().first;
        }

};

// Opt Policy uses a ordered list of further seem flows
// These flows need to be evicted
template<typename lookup_table_t, typename  stats_table_t>
using OPTPolicy = LRUPolicy<lookup_table_t, stats_table_t>;

template<typename lookup_table_t, typename  stats_table_t>
class LFUPolicy final: public Policier<lookup_table_t,stats_table_t>
{

    public:
        using policer_t =  Policier<lookup_table_t, stats_table_t>;

        LFUPolicy(const lookup_table_t& lookup_table,
                        const stats_table_t& stats_table) :
                        policer_t(lookup_table, stats_table)
        {}

        virtual FiveTuple select_replacement_victim() const override
        {
            // Get the least recently used entry.
            // TUple + value associe
            // 1. Get stats
            return this->stats_table_.back().first;
        }

};
#endif
