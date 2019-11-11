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
        using lookup_mem_t = std::unordered_map<FiveTuple, Lookup_Value>;
        //using table_size_t = Lookup_Size;
        using table_value_t = Lookup_Value;
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

        // No bound check for inserting...
        // Make sure the controller checks that
        auto insert (const FiveTuple& five_tuple, const Lookup_Value& value) {
            std::unique_lock lock(mutex_);
            std::cout << "Inserting: " << five_tuple;
            lookup_table_.insert({ five_tuple, value });
            if (occupancy_ < LOOKUP_MEM_SIZE) {
                occupancy_++;
            }
            return true;
        }

        auto is_full() const {
            std::shared_lock lock(mutex_);
            return occupancy_ >= LOOKUP_MEM_SIZE;
        }

        auto remove (const FiveTuple& five_tuple) {
            if (find(five_tuple) != lookup_table_.end()) {
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

        auto& data () {   // Raw Data
            std::unique_lock lock(mutex_);
            return lookup_table_;
        }

        LookupTable () {}
        LookupTable (const std::size_t) {}
        LookupTable (const lookup_mem_t& lookup_table) : lookup_table_(lookup_table), occupancy_(lookup_table.size()){}

    private:
        lookup_mem_t lookup_table_;
        mutable std::shared_mutex mutex_;
        size_t occupancy_ = 0;

};
#endif
