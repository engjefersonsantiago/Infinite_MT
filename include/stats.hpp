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

enum class CacheType : uint8_t {
    LRU,
    LFU,
    LRFU,
    RANDOM
};

template<size_t Stats_Size, typename Stats_Value, typename Stats_Container>
class CacheStats {

    public:
        // Constants
        static constexpr auto STATS_MEM_SIZE = Stats_Size;

        // Writter
        virtual void update_stats (const FiveTuple& five_tuple, Stats_Value& updated_stats) =0;

        // Read the whole stats
        auto& get_stats () const {
            std::shared_lock lock(mutex_);  // Needs to be unique, cause the Dataplane should not
                                            // modify the stats during controller reading
            return stats_container_;        //TODO: If using circular_buffer, not thread safe.
        }

        // Clear available thru container.clear()
    protected:
        // Think about a generic container for the stats...
        Stats_Container stats_container_ { Stats_Size };
        mutable std::shared_mutex mutex_;
        size_t capacity_ = 0;

};

// LRU Cache stats specialization
template<typename T>
using LRUContainer = boost::circular_buffer<std::pair<FiveTuple, T>>;

template<size_t Stats_Size, typename Stats_Value>
class LRUCacheStats final : public CacheStats<Stats_Size, Stats_Value, LRUContainer<Stats_Value>> {
    public:
        virtual void update_stats (const FiveTuple& five_tuple, Stats_Value& updated_stats) {
            std::unique_lock lock(this->mutex_);
            this->stats_container_.push_back({ five_tuple, updated_stats });
        }
};

// LFU Cache stats specialization
template<typename T>
using LFUContainer = SortedContainer<std::pair<FiveTuple, T>>;

template<size_t Stats_Size, typename Stats_Value>
class LFUCacheStats final : public CacheStats<Stats_Size, Stats_Value, LFUContainer<Stats_Value>> {

    public:
        virtual void update_stats (const FiveTuple& five_tuple, Stats_Value& updated_stats) {
            std::unique_lock lock(this->mutex_);
            auto tuple_compare = [&five_tuple = std::as_const(five_tuple)](const auto& elem) { 
                return elem.first == five_tuple; 
            };
            auto value_sort = [](auto a, auto b) { return a.second > b.second; };
            auto value_compare = [](auto a, auto b) { return std::max(a.second, b.second); };

            auto found = this->stats_container_.find({five_tuple, 0}, tuple_compare);
            if (found !=  this->stats_container_.end())
            {
                *found = {found->first, found->second + updated_stats};
                this->stats_container_.sort(value_sort);
            } else
            {
                this->stats_container_.insert({five_tuple, updated_stats}, value_sort, value_compare);
            }
        }
};

// LFU implemented as a streaming algorithm... we keep most active flows (more bandwidth) in cache
// LRU might be implemented as a static circular buffer. Impact of duplicates??
// Random can be implemented directed in the controller
// LRFU can be implemented as a timed streaming algorithm...

#endif
