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
        static constexpr auto LOOKUP_MEM_SIZE = Lookup_Size; // -1 means infinite memory

        auto begin() {
//            std::unique_lock lock(mutex_);
            return lookup_table_.begin();
        }

        auto begin() const {
//            std::unique_lock lock(mutex_);
            return lookup_table_.begin();
        }

        auto end() const {
//            std::unique_lock lock(mutex_);
            return lookup_table_.end();
        }
        
        auto end() {
//            std::unique_lock lock(mutex_);
            return lookup_table_.end();
        }


        auto find (const FiveTuple& five_tuple) const {
//            std::shared_lock lock(mutex_);
            return lookup_table_.find(five_tuple);
        }

        // No bound check for inserting...
        // Make sure the controller checks that
        auto insert (const FiveTuple& five_tuple, const Lookup_Value& value) {
            if (find(five_tuple) == lookup_table_.end() && !is_full()) {
//                std::unique_lock lock(mutex_);
                //std::cout << "Inserting: " << five_tuple << "Current occupancy: " << occupancy_ << '\n';
                lookup_table_.insert({ five_tuple, value });
                occupancy_++;
            }
//            //std::unique_lock lock(mutex_);
            //if (occupancy_ < LOOKUP_MEM_SIZE) {
            //    occupancy_++;
            //}
            return true;
        }

        auto is_full() const {
//            std::shared_lock lock(mutex_);
            return lookup_table_.size() >= LOOKUP_MEM_SIZE && LOOKUP_MEM_SIZE != size_t(-1);
        }

        auto remove (const FiveTuple& five_tuple) {
//            std::unique_lock lock(mutex_);
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
//            std::shared_lock lock(mutex_);
            //return occupancy_;
            return lookup_table_.size();
        }

        auto& data () const {   // Raw Data
//            std::shared_lock lock(mutex_);
            return lookup_table_;
        }

        auto& data () {   // Raw Data
//            std::unique_lock lock(mutex_);
            return lookup_table_;
        }

//        auto indexed_iter (std::size_t idx) {
//            std::unique_lock lock(mutex_);
//            return std::next(lookup_table_.begin(), idx);
//        }
//
        auto indexed_iter (std::size_t idx) const {
//            std::shared_lock lock(mutex_);
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
//        mutable std::shared_mutex mutex_;
        size_t occupancy_ = 0;

};
#endif
