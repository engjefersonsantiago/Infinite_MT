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

#ifndef __LOOKUP_TABLE__
#define __LOOKUP_TABLE__

#include "pkt_common.hpp"
template<size_t Lookup_Size, typename Lookup_Value>
class LookupTable {

    public:
        // Constants
        static constexpr auto LOOKUP_MEM_SIZE = Lookup_Size;

        auto lookup (const FiveTuple& five_tuple) const {
            std::shared_lock lock(mutex_);
            return lookup_table_.find(five_tuple) != lookup_table_.end();
        }

        auto add_key (const FiveTuple& five_tuple, const Lookup_Value& value) {
            std::unique_lock lock(mutex_);
            if (capacity_ < LOOKUP_MEM_SIZE) {
                capacity_++;
                lookup_table_.insert(five_tuple, value);
                return true;
            } else {
                return false;
            }
        }

        auto delete_key (const FiveTuple& five_tuple) {
            std::unique_lock lock(mutex_);
            if (lookup(five_tuple)) {
                capacity_--;
                lookup_table_.erase(five_tuple);
                return true;
            } else {
                return false;
            }
        }

        auto replace_key (const FiveTuple& old_tuple, const FiveTuple& new_tuple, const Lookup_Value& new_value) {
            // No need for lock here
            return (delete_key(old_tuple)) ? add_key(new_tuple, new_value) : false;
        }

        auto get_capacity () const {
            std::shared_lock lock(mutex_);
            return capacity_;
        }

    private:
        std::unordered_map<FiveTuple, Lookup_Value> lookup_table_;
        mutable std::shared_mutex mutex_;
        size_t capacity_ = 0;

};
#endif
