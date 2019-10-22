// File: lookup_table.hpp
// Purpose: Lookup table class
// Author: jeferson.silva@polymtl.ca

// STL libs
#include <cstdint>
#include <array>
#include <tuple>
#include <mutex>
#include <unordered_map>
#include <shared_mutex>

#ifndef __STATS_LIB__
#define __STATS_LIB__
/*
* Shared With Cache / Controller Class and Policy.
*/

#include "pkt_common.hpp"
template<size_t Cache_Size, typename Stats_Value>
class CacheStats{

    public:
        // Constants
        static constexpr auto STATS_MEM_SIZE = Cache_Size;

        auto& lookup (const FiveTuple& five_tuple)  {
            std::shared_lock lock(mutex_);
            return StatsContainer.find(five_tuple);
        }


        /* Used only by external class only */
        auto add_key (const FiveTuple& five_tuple, const Stats_Value& value) {
            std::unique_lock lock(mutex_);
            if (capacity_ < Cache_Size) {
                capacity_++;
                StatsContainer.insert(five_tuple, value);
                return true;
            } else {
                return false;
            }
        }

        /* Used only by external class only */
        auto delete_key (const FiveTuple& five_tuple) {
            std::unique_lock lock(mutex_);
            if (lookup(five_tuple)) {
                capacity_--;
                StatsContainer.erase(five_tuple);
                return true;
            } else {
                return false;
            }
        }

        auto replace_key (const FiveTuple& old_tuple, const FiveTuple& new_tuple, const Stats_Value& new_value) {
            // No need for lock here
            return (delete_key(old_tuple)) ? add_key(new_tuple, new_value) : false;
        }

        auto get_capacity () const {
            std::shared_lock lock(mutex_);
            return capacity_;
        }

    private:
        std::unordered_map<FiveTuple, Stats_Value> StatsContainer;
        mutable std::shared_mutex mutex_;
        size_t capacity_ = 0;

};
#endif
