// File: stats.hpp
// Purpose: Cache stats base and specialized classes
// Author: thibaut.stimpfling@polymtl.ca

// STL libs
#include <cstdint>
#include <array>
#include <tuple>
#include <mutex>
#include <unordered_map>
#include <shared_mutex>
#include <utility>

#ifndef __STATS_LIB__
#define __STATS_LIB__
/*
* Shared With Cache / Controller Class and Policy.
*/

#include "pkt_common.hpp"
#include "sorted_container.hpp"
#include "multi_index_sorted_container.hpp"

/*
*
* Stats Class Used by a Cache.
*
* Interfaces with the cache, the controller and the policier.
*
*/

template<size_t Stats_Size, typename Stats_Value, typename Stats_Container>
class CacheStats
{

    public:
        using stats_tuple = std::pair<FiveTuple, Stats_Value>;
        // Constants
        static constexpr auto STATS_MEM_SIZE = Stats_Size;

        // Writter
        virtual void update_stats (const FiveTuple& five_tuple, const Stats_Value& updated_stats) =0;

        // Read the whole stats
        auto& get_stats () { return stats_container_; }

        // CTOR
        CacheStats () {}

    protected:
        // Think about a generic container for the stats...
        Stats_Container stats_container_;

};

// LRU Cache stats specialization
template<std::size_t N, typename T>
using LRUContainer = MultiIndexSortedContainer<N, FiveTuple, T, std::less<T>>;

// Duplicate code betwen LFU and LRU... Improve it
template<size_t Stats_Size, typename Stats_Value>
class LRUCacheStats final : public CacheStats<Stats_Size, Stats_Value, LRUContainer<Stats_Size, Stats_Value>>
{
    public:

        virtual void update_stats (const FiveTuple& five_tuple, const Stats_Value& updated_stats) override
        {
            if (updated_stats != 0)
            {
                auto found = this->stats_container_.find(five_tuple);
                if (found !=  this->stats_container_.end())
                {
                    //std::cout << "B Modified: " <<  this->stats_container_.size() << '\n';
                    this->stats_container_.modify(found, updated_stats);
                    //std::cout << "A Modified: " <<  this->stats_container_.size() << '\n';
                } else
                {
                    this->stats_container_.insert({ five_tuple, updated_stats } );
                }
            }
        }
};

// LFU Cache stats specialization
template<std::size_t N, typename T>
using LFUContainer = MultiIndexSortedContainer<N, FiveTuple, T, std::less<T>>;

template<size_t Stats_Size, typename Stats_Value>
class LFUCacheStats final : public CacheStats<Stats_Size, Stats_Value, LFUContainer<Stats_Size, Stats_Value>>
{
    public:
        virtual void update_stats (const FiveTuple& five_tuple, const Stats_Value& updated_stats) override
        {
            if (updated_stats != 0)
            {
                auto found = this->stats_container_.find(five_tuple);
                if (found !=  this->stats_container_.end())
                {
                    //std::cout << "B Modified: " <<  this->stats_container_.size() << '\n';
                    this->stats_container_.modify(found, found->value + updated_stats);
                    //std::cout << "A Modified: " <<  this->stats_container_.size() << '\n';
                } else
                {
                    this->stats_container_.insert({ five_tuple, updated_stats });
                }
            }
        }
};

// LFU Cache stats specialization
template<std::size_t N, typename T>
using NXUContainer = MultiIndexSortedContainer<N, FiveTuple, T, std::greater<T>>;

template<size_t Stats_Size, typename Stats_Value>
class NXUCacheStats final : public CacheStats<Stats_Size, Stats_Value, NXUCacheStats<Stats_Size, Stats_Value>>
{
    public:
        virtual void update_stats (const FiveTuple& five_tuple, const Stats_Value& updated_stats) override
        {
            if (updated_stats != 0)
            {
                auto found = this->stats_container_.find(five_tuple);
                if (found !=  this->stats_container_.end())
                {
                    //std::cout << "B Modified: " <<  this->stats_container_.size() << '\n';
                    this->stats_container_.modify(found, found->value + updated_stats);
                    //std::cout << "A Modified: " <<  this->stats_container_.size() << '\n';
                } else
                {
                    this->stats_container_.insert({ five_tuple, updated_stats });
                }
            }
        }
};

// Optimal cache stats specialization
// Calculate offline the list of flows to be evicted
// Store teh into a queue
template<typename T>
using OPTContainer = SortedContainer<std::pair<FiveTuple, T>>;

template<size_t Stats_Size, typename Stats_Value>
class OPTCacheStats final : public CacheStats<Stats_Size, Stats_Value, OPTContainer<Stats_Value>>
{
    public:
        using cache_stats_t = CacheStats<Stats_Size, Stats_Value, OPTContainer<Stats_Value>>;
        using opt_container_t = OPTContainer<Stats_Value>;

    private:
        opt_container_t optimal_replace_list () const {
            // Open the trace
            //
        }

    public:

        virtual void update_stats (const FiveTuple& five_tuple, const Stats_Value& updated_stats) override
        {
        }
};

#endif
