// File: static_hash_map.hpp
// Purpose: Static hash map implementation
// Author: jeferson.silva@polymtl.ca

// STL libs
#include <cstdint>
#include <array>
#include <tuple>

#ifndef __STATIC_HASH_MAP__
#define __STATIC_HASH_MAP__

// StaticHashMap class
// Params:
// Size: size of the hash map, size_t
// Key: type of the key, typename (must be hashable and comparable)
// Value: type of the value, typename
// Hash: callable of a custom hash function, typename
// Compare: callable of a custom compare function, typename


// What is the purpose of this hash table? Why not use a regular unordered_map and check the number of elements inserted?
// We need a unordered_map where the maximal number of keys is fixed. Could used .size() method.
template <size_t Size,
         typename Key,
         typename Value,
         typename Hash = std::hash<Key>,
         typename Compare = std::equal_to<Key>>
class StaticHashMap {
    public:


        // Types
        using key_value_t = std::pair<Key, Value>;
        using hash_table_t = std::array<key_value_t, Size>;

        // Constants
        static constexpr auto SIZE = Size;

        // Iterators
        auto begin() { return hash_table_.begin(); }
        auto end() { return hash_table_.end(); }

        // Must be modified accordingly in case of collision
        inline size_t get_index (const Key& key) const {
            return hf(key) % Size;
        }

        auto compare (const Key& key) const {
            return cf(key, hash_table_[get_index(key)].first);
        }

        // Accessors
        // Conflict resolution not supported.
        // If a is already inserted, a new key b can overide a.
        // Proposed to use a linked list to track down the number of elements inserted in the hash table.
        // Hash Table bucket holds a vector. 
        auto& operator[](const Key& key) {
            hash_table_[get_index(key)].first = key;
            return hash_table_[get_index(key)].second;
        }

        const auto& operator[](const Key& key) const {
            return hash_table_[get_index(key)];
        }

        auto find (const Key& key) {
            return (compare(key)) ? &hash_table_[get_index(key)] : hash_table_.end();
        }

    private:
        // Hash table
        hash_table_t hash_table_;
        // Tracks the hash table capacity.
        size_t num_keys_inserted_;
        // Hash and comparsion functions
        const Hash hf = Hash();
        const Compare cf = Compare();

}; // StaticHashMap

#endif //__STATIC_HASH_MAP__
