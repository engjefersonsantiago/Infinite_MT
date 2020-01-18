// STL
#include <initializer_list>
#include <algorithm>

// Boost
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#ifndef MULTI_INDEX_SORTED_CONTAINER
#define MULTI_INDEX_SORTED_CONTAINER

using boost::multi_index_container;
using namespace boost::multi_index;

// Hashed and Sorted unordered container
// Key: key to access the data (must be hashable)
// Value: value to sort the data
// CompareHigher: Higher order compare function
// CompareLower: lower order compare function
template<std::size_t Size, typename Key, typename Value, typename CompareHigher>
class MultiIndexSortedContainer
{
    private:
        struct KeyValue {
            Key key;
            Value value;
        };

        struct ChangeValue
        {
            ChangeValue(const Value& new_value) : new_value_(new_value) {}
            void operator()(KeyValue& key_val) { key_val.value = new_value_; }

            private:
                Value new_value_;
        };

        using KeyValContainer = multi_index_container<
            KeyValue,
            indexed_by<
               hashed_unique<BOOST_MULTI_INDEX_MEMBER(KeyValue, Key, key)>,
               ordered_non_unique<BOOST_MULTI_INDEX_MEMBER(KeyValue, Value, value), CompareHigher>
            >
        >;

        using iterator = typename KeyValContainer::iterator;

        static constexpr auto MAX_SIZE = Size;

        KeyValContainer key_value_container_;
        std::size_t occupancy_ = 0;

    public:
        // CTORs
        MultiIndexSortedContainer(){}
        // Not working
        MultiIndexSortedContainer(std::initializer_list<KeyValue> key_value) : key_value_container_(key_value) {}

        // Indexing by type does not work...
        auto find (const Key& key) const
        {
            return get<0>(key_value_container_).find(key);
        }

        auto highest_order()
        {
            auto front = get<1>(key_value_container_).begin();
            return find(front->key);
        }

        auto lowest_order()
        {
            auto front = get<1>(key_value_container_).rbegin();
            return find(front->key);
        }

        bool insert (const KeyValue& key_val)
        {
            if (key_value_container_.size() < MAX_SIZE)
            {
                return key_value_container_.insert(key_val).second;
            } else
            {
                return key_value_container_.replace(lowest_order(), key_val);
            }
        }

        void erase (iterator key_val)
        {
            if (key_value_container_.size() > 0)
            {
                key_value_container_.erase(key_val);
            }
        }

        auto modify (iterator key_val, const Value& new_value)
        {
            return key_value_container_.modify(key_val, ChangeValue(new_value));
        }

        auto replace (iterator key_val, const KeyValue& new_key_value)
        {
            return key_value_container_.replace(key_val, new_key_value);
        }

        auto begin() { return key_value_container_.begin(); }
        auto end() { return key_value_container_.end(); }

        auto size() const { return key_value_container_.size(); }

};

#endif
