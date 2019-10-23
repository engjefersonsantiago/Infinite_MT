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

        auto begin() { 
            std::shared_lock lock(mutex_);
            return lookup_table_.begin();
        }
        
        auto end() { 
            std::shared_lock lock(mutex_);
            return lookup_table_.end();
        }

        auto find (const FiveTuple& five_tuple) const {
            std::shared_lock lock(mutex_);
            return lookup_table_.find(five_tuple);
        }

        auto insert (const FiveTuple& five_tuple, const Lookup_Value& value) {
            std::unique_lock lock(mutex_);
            if (occupancy_ < LOOKUP_MEM_SIZE) {
                occupancy_++;
                lookup_table_.insert({ five_tuple, value });
                return true;
            } else {
                return false;
            }
        }

        auto remove (const FiveTuple& five_tuple) {
            if (find(five_tuple)) {
                std::unique_lock lock(mutex_);
                occupancy_--;
                lookup_table_.erase(five_tuple);
                return true;
            } else {
                return false;
            }
        }

        auto replace (const FiveTuple& old_tuple, const FiveTuple& new_tuple, const Lookup_Value& new_value) {
            // No need for lock here
            return (remove(old_tuple)) ? insert(new_tuple, new_value) : false;
        }

        auto occupancy () const {
            std::shared_lock lock(mutex_);
            return occupancy_;
        }

        auto& data () const {   // Raw Data
            std::shared_lock lock(mutex_);
            return lookup_table_;
        }

        LookupTable () {}
        LookupTable (const std::size_t) {}

    private:
        std::unordered_map<FiveTuple, Lookup_Value> lookup_table_;
        mutable std::shared_mutex mutex_;
        size_t occupancy_ = 0;

};
#endif
