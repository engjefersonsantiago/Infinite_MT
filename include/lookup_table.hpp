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
        using table_value_t = Lookup_Value;
        // Constants
        static constexpr auto LOOKUP_MEM_SIZE = Lookup_Size;

        auto begin() {
            return lookup_table_.begin();
        }

        auto begin() const {
            return lookup_table_.begin();
        }

        auto end() const {
            return lookup_table_.end();
        }
        
        auto end() {
            return lookup_table_.end();
        }


        auto find (const FiveTuple& five_tuple) const {
            return lookup_table_.find(five_tuple);
        }

        // No bound check for inserting...
        // Make sure the controller checks that
        auto insert (const FiveTuple& five_tuple, const Lookup_Value& value) {
            if (find(five_tuple) == lookup_table_.end() && !is_full()) {
                //std::cout << "Inserting: " << five_tuple << "Current occupancy: " << occupancy_ << '\n';
                lookup_table_.insert({ five_tuple, value });
                occupancy_++;
            }
            //std::unique_lock lock(mutex_);
            //if (occupancy_ < LOOKUP_MEM_SIZE) {
            //    occupancy_++;
            //}
            return true;
        }

        auto is_full() const {
            return lookup_table_.size() >= LOOKUP_MEM_SIZE;
        }

        auto remove (const FiveTuple& five_tuple) {
            if (lookup_table_.find(five_tuple) != lookup_table_.end())
            {
                //std::cout << "Removing: " << five_tuple << "Current occupancy: " << occupancy_ << '\n';;
                occupancy_--;
                lookup_table_.erase(five_tuple);
                return true;
            } else
            {
                return false;
            }
        }

        auto replace (const FiveTuple& old_tuple, const FiveTuple& new_tuple, const Lookup_Value& new_value) {
            // No need for lock here
            return (remove(old_tuple)) ? insert(new_tuple, new_value) : false;
        }

        auto occupancy () const {
            //return occupancy_;
            return lookup_table_.size();
        }

        auto& data () const {   // Raw Data
            return lookup_table_;
        }

        auto& data () {   // Raw Data
            return lookup_table_;
        }

        auto indexed_iter (std::size_t idx) const {
            auto it = lookup_table_.begin(); 
            return *std::next(it, idx);
            /*std::size_t i = 0;
            for (const auto it : lookup_table_) {
                if (i++ == idx)
                    return it;
            }
            return *lookup_table_.begin();*/
        }

        LookupTable () {}
        LookupTable (const std::size_t) {}
        LookupTable (const lookup_mem_t& lookup_table) : lookup_table_(lookup_table), occupancy_(lookup_table.size()){}

    private:
        lookup_mem_t lookup_table_;
        size_t occupancy_ = 0;

};
#endif
