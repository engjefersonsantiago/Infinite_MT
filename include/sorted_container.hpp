#include<algorithm>
#include<iostream>
#include<memory>
#include<vector>

#ifndef __SORTED_CONTAINER__
#define __SORTED_CONTAINER__

template<typename Type>
class SortedContainer {
    private:
        std::vector<Type> data_;
        const size_t size_;
        size_t occupancy_;
        mutable std::shared_mutex mutex_;

    public:
        SortedContainer (const size_t size) :
            data_(size),
            size_(size), occupancy_(0) {}

        auto begin() { 
            std::unique_lock lock(mutex_);
            return data_.begin();
        }
        auto end() {
            std::unique_lock lock(mutex_);
            return data_.end();
        }
        auto clear() { 
            std::unique_lock lock(mutex_);
            return data_.clear();
        }

        template<typename Compare>
        auto find(const Type& elem, Compare&& c) {
            std::unique_lock lock(mutex_);
            return std::find(data_.begin(), data_.end(), c);
        }

        template<typename Sort>
        void sort (Sort&& s) {
            std::unique_lock lock(mutex_);
            std::sort(data_.begin(), data_.end(), s);
        }

        template<typename Sort, typename Compare>
        void insert (const Type& element, Sort&& s, Compare&& c) {
            std::unique_lock lock(mutex_);
            {
                if (occupancy_ < size_) {
                    data_[occupancy_++] = element;
                } else {
                    // Compare the back element with the new element
                    data_.back() = c(data_.back(), element);
                }
            }
            // Sort using the predicate s
            sort(s);
        }
};

#endif
