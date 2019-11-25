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
            //data_(size),
            data_{},
            size_(size), occupancy_(0) {}

        auto erase (typename std::vector<Type>::const_iterator it)
        {
            std::unique_lock lock(mutex_);
            {
            return data_.erase(it);
            }
        }

        auto occupancy () const
        {
            std::unique_lock lock(mutex_);
            return occupancy_;
        }

        auto begin() {
            std::unique_lock lock(mutex_);
            return data_.begin();
        }
        auto end() {
            std::unique_lock lock(mutex_);
            return data_.end();
        }

        auto begin() const {
            std::unique_lock lock(mutex_);
            return data_.begin();
        }
        auto end() const {
            std::unique_lock lock(mutex_);
            return data_.end();
        }

        auto clear() {
            std::unique_lock lock(mutex_);
            return data_.clear();
        }

        auto& front() {
            std::unique_lock lock(mutex_);
            return data_.front();
        }
        auto& back() {
            std::unique_lock lock(mutex_);
            return data_.back();
        }

        auto& front() const {
            std::unique_lock lock(mutex_);
            return data_.front();
        }
        auto& back() const {
            std::unique_lock lock(mutex_);
            return data_.back();
        }

        template<typename Compare>
        auto find_if(Compare&& c) {
            std::unique_lock lock(mutex_);
            return std::find_if(data_.begin(), data_.end(), c);
        }

        auto& data() {
            std::unique_lock lock(mutex_);
            return data_;
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
                //std::cout << occupancy_ << '\n';
                if (occupancy_ < size_) {
                    //if ((occupancy_ > 0) && data_.front() == Type{})
                    //    data_.front() = element;
                    //else
                    //{
                        occupancy_++;
                        data_.push_back(element);
                    //}
                } else {
                    // Compare the back element with the new element
                    //data_.back() = c(data_.back(), element);
                    //if (data_.front() == Type{})
                    //    data_.front() = element;
                    //else
                        data_.back() = element;
                    //data_[occupancy_-1] = element;
                    //std::cout << back().first << '\n';
                }
            }
            // Sort using the predicate s
            // Commented out because it's trowing a deadlock exception
            //sort(s);
        }
};

#endif
