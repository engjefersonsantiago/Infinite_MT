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
        virtual void update_stats (const FiveTuple& five_tuple, Stats_Value& updated_stats) =0;

        // Read the whole stats
        auto& get_stats ()
        {
            std::unique_lock lock(mutex_);  // Needs to be unique, cause the Dataplane should not
                                            // modify the stats during controller reading
            return stats_container_;        //TODO: If using circular_buffer, not thread safe.
        }

        auto& back () const
        {
            std::unique_lock lock(mutex_);
            return stats_container_.back();
        }
        auto& front () const  
        {
            std::unique_lock lock(mutex_);
            return stats_container_.front();
        }


        auto& back () 
        {
            std::unique_lock lock(mutex_);
            return stats_container_.back();
        }
        auto& front () 
        {
            std::unique_lock lock(mutex_);
            return stats_container_.front();
        }

        // Clear available thru container.clear()

        // CTOR
        CacheStats () {}
        CacheStats (const Stats_Container& stats_container) : 
            stats_container_(stats_container),
            capacity_(stats_container.size())
        {}

    protected:
        // Think about a generic container for the stats...
        Stats_Container stats_container_ { Stats_Size };
        mutable std::shared_mutex mutex_;
        size_t capacity_ = 0;

};

// LRU Cache stats specialization
template<typename T>
using LRUContainer = SortedContainer<std::pair<FiveTuple, T>>;

// Duplicate code betwen LFU and LRU... Improve it
template<size_t Stats_Size, typename Stats_Value>
class LRUCacheStats final : public CacheStats<Stats_Size, Stats_Value, LRUContainer<Stats_Value>> 
{
    public:

        virtual void update_stats (const FiveTuple& five_tuple, Stats_Value& updated_stats) override
        {
            std::unique_lock lock(this->mutex_);
            if (updated_stats != 0)
            {
                auto tuple_compare = [=](const auto& elem) {
                    return elem.first == five_tuple;
                };
                auto value_sort = [](auto a, auto b) { return a.second < b.second; };
                auto value_compare = [](auto a, auto b) { return (a.second < b.second) ? a : b; };

                auto found = this->stats_container_.find_if(tuple_compare);
                if (found !=  this->stats_container_.end())
                {
                    *found = std::make_pair(found->first, updated_stats);
                } else
                {
                    this->stats_container_.insert(std::make_pair(five_tuple, updated_stats), value_sort, value_compare);
                }
                this->stats_container_.sort(value_sort);
                //std::cout << "------------------------\n";
                //for (auto i : this->stats_container_) {
                //    std::cout << i.first << " , " << i.second << '\n';
                //} 
                //std::cout << this->stats_container_.back().first << '\n';
                //int i;
                //std::cin >> i;
            }
        }
};

// LFU Cache stats specialization
template<typename T>
using LFUContainer = SortedContainer<std::pair<FiveTuple, T>>;

template<size_t Stats_Size, typename Stats_Value>
class LFUCacheStats final : public CacheStats<Stats_Size, Stats_Value, LFUContainer<Stats_Value>>
{

    public:
        virtual void update_stats (const FiveTuple& five_tuple, Stats_Value& updated_stats) override
        {
            std::unique_lock lock(this->mutex_);
            if (updated_stats != 0)
            {
                auto tuple_compare = [=](const auto& elem) {
                    return elem.first == five_tuple;
                };
                //auto value_sort = [](auto a, auto b) { return a.second < b.second; };
                auto value_sort = [](auto a, auto b) { return a.second > b.second; };
                auto value_compare = [](auto a, auto b) { return (a.second < b.second) ? a : b; };

                auto found = this->stats_container_.find_if(tuple_compare);
                if (found !=  this->stats_container_.end())
                {
                    *found = std::make_pair(found->first, found->second + updated_stats);
                } else
                {
                    this->stats_container_.insert(std::make_pair(five_tuple, updated_stats + this->stats_container_.front().second), value_sort, value_compare);
                }
                //this->stats_container_.sort(value_sort);

                //std::cout << "------------------------\n";
                //for (auto i : this->stats_container_) {
                //    std::cout << i.first << " , " << i.second << '\n';
                //} 

                std::make_heap(this->stats_container_.begin(), this->stats_container_.end(), value_sort); 
                
                //std::cout << "------------------------\n";
                //for (auto i : this->stats_container_) {
                //    std::cout << i.first << " , " << i.second << '\n';
                //} 
                //std::cout << this->stats_container_.back().first << '\n';
                //int i;
                //std::cin >> i;
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

        virtual void update_stats (const FiveTuple& five_tuple, Stats_Value& updated_stats) override
        {
        }
};

#endif
