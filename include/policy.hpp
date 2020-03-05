#ifndef __POLICY__
#define __POLICY__

#include <random>
#include <algorithm>

#include "pkt_common.hpp"
#include "controller.hpp"
#include "lookup_table.hpp"
#include "stats.hpp"
#include "buffered_reader.hpp"
#include "priority_deque/priority_deque.hpp"
// Quels sont les composants communs a une politique de cache.

// Base sur un input (Cache Stats) + Event (Cache Miss), identifie l'entree a ajouter
// (lookup container controller) et l entree a retirer.
// Interface avec le controller.
// Policy doit pouvoir s

// Policer is specialized for a given policy.
// Currently assume that L1 cache enries are updated following a cache miss.

template<typename lookup_table_t, typename  stats_table_t, typename promo_stats_table_t>
class Policer{

    public:

        Policer(const lookup_table_t& lookup_table,
                    stats_table_t& stats_table,
                    promo_stats_table_t& promo_stats_table) :
                    lookup_table_{lookup_table},
                    stats_table_{stats_table},
                    promo_stats_table_{promo_stats_table} {}

        virtual FiveTuple select_replacement_victim(const FiveTuple& five_tuple, size_t timestamp) =0;

        auto& stats_table () { return stats_table_; }
        const auto& lookup_table () const { return lookup_table_; }

        FiveTuple select_promotion_candidate() {
            auto tuple_to_promote = this->promo_stats_table_.get_stats().highest_order();
            auto selected = tuple_to_promote->key;
            promo_stats_table_.get_stats().erase(tuple_to_promote);
            return selected;

        }
        //FiveTuple select_promotion_candidate() {
        //    auto ret =  promo_stats_table_.get_stats().front().first;
        //    promo_stats_table_.get_stats().front().second = 0;
        //    return ret;
        //}
        
        auto& promo_stats_table () { return promo_stats_table_; }

    protected:
        const lookup_table_t& lookup_table_;
        stats_table_t& stats_table_;
        promo_stats_table_t& promo_stats_table_;

};

template<typename lookup_table_t, typename  stats_table_t, typename promo_stats_table_t>
class RandomPolicy final: public Policer<lookup_table_t,stats_table_t,promo_stats_table_t>
{

    public:
        using policer_t = Policer<lookup_table_t, stats_table_t,promo_stats_table_t>;

    private:

        // Hide the random generation mechanism
        std::size_t random_number_generation() const
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, lookup_table_t::LOOKUP_MEM_SIZE - 1);
            return dis(gen);
        }

    public:

        RandomPolicy(const lookup_table_t& lookup_table,
                        stats_table_t& stats_table,
                        promo_stats_table_t& promo_stats_table) :
                        policer_t(lookup_table, stats_table, promo_stats_table)
        {}

        virtual FiveTuple select_replacement_victim(const FiveTuple& , size_t) override
        {
            // From https://stackoverflow.com/questions/27024269/select-random-element-in-an-unordered-map
            const auto tuple = this->lookup_table_.indexed_iter(random_number_generation()).first;
            debug(std::cout << "Selected " << tuple << " for replacement\n";)
            return tuple;
        }
};

template<typename lookup_table_t, typename  stats_table_t, typename promo_stats_table_t>
class LRUPolicy final: public Policer<lookup_table_t,stats_table_t,promo_stats_table_t>
{

    public:
        using policer_t =  Policer<lookup_table_t, stats_table_t,promo_stats_table_t>;

        LRUPolicy(const lookup_table_t& lookup_table,
                        stats_table_t& stats_table,
                        promo_stats_table_t& promo_stats_table) :
                        policer_t(lookup_table, stats_table, promo_stats_table)
        {}

        virtual FiveTuple select_replacement_victim(const FiveTuple& five_tuple, size_t timestamp) override
        {
            // Get the least recently used entry.
            auto tuple_to_remove = this->stats_table_.get_stats().highest_order();
            auto selected = tuple_to_remove->key;

            this->stats_table_.get_stats().replace(
                tuple_to_remove,
                {
                    five_tuple,
                    timestamp
                }
            );

            debug(
            std::cout << "---------------------------\n";
            std::cout << "Lookup table contents\n";
            std::cout << "---------------------------\n";
            for(const auto& [key, _] : this->lookup_table_)
            {
                std::cout <<  key << '\n';
            }
            std::cout << "---------------------------\n";
            std::cout << "Stats table contents\n";
            std::cout << "---------------------------\n";
            for(const auto& [key, val] : this->stats_table_.get_stats())
            {
                std::cout <<  key <<  ", " << val << '\n';
            }
            std::cout << "---------------------------\n";
            std::cout << "Key to remove " << selected << '\n';
            std::cout << "---------------------------\n";
            )

            return selected;
        }


};

// Opt Policy uses a ordered list of further seem flows
// These flows need to be evicted
template<typename lookup_table_t, typename  stats_table_t, typename promo_stats_table_t>
class OPTPolicy final : public Policer<lookup_table_t,stats_table_t,promo_stats_table_t>
{
    //TODO: validate when we enqueue that the current timestamp is also pushed. Need a global visibility on the timestamp (i.e. a scheduler!)
    public:
        using policer_t =  Policer<lookup_table_t, stats_table_t,promo_stats_table_t>;
        using fivetuple_history_t = std::unordered_map<FiveTuple, std::pair<std::size_t, std::vector<std::size_t>>>;

        struct ReplacementQueueElement {
            std::size_t next_usage_time;
            fivetuple_history_t::iterator fivetuple_history_entry;

            ReplacementQueueElement(std::size_t n, const fivetuple_history_t::iterator& i) : next_usage_time{n}, fivetuple_history_entry{i} {}
            bool operator< (const ReplacementQueueElement& other) const { return next_usage_time < other.next_usage_time; }
        };

        using replacement_queue_t = boost::container::priority_deque<ReplacementQueueElement>;

        OPTPolicy(const lookup_table_t& lookup_table,
                        stats_table_t& stats_table,
                        promo_stats_table_t& promo_stats_table,
                         const std::string& file_name) :
                        policer_t(lookup_table, stats_table, promo_stats_table), 
                        pcap_pre_file_name{file_name}
        {
             //build_five_tuple_history();
        }

        void build_five_tuple_history(){
            BufferedReader<> reader(pcap_pre_file_name, std::ios::binary);

            if (!reader.getStream().is_open())
            {
                std::cout << "Error opening the pcap file\n";

            }

            five_tuple_history.reserve(2500000); // Only for a small load time optimization.

            std::size_t pkts = 0;
            for (const ParsedPacket* parsedPacket; (parsedPacket = reader.peekAs<ParsedPacket*>()) != nullptr; reader += sizeof(ParsedPacket)) {
                // Enqueue FiveTuple
                auto& fivetuple_history_entry = five_tuple_history[parsedPacket->five_tuple];
                if (fivetuple_history_entry.second.empty()) fivetuple_history_entry.second.reserve(5); // Only for a small load time optimization.  
                fivetuple_history_entry.first = 0;
                fivetuple_history_entry.second.push_back(++pkts);
            }

            std::cout << "------ Content of the five tuple history ------\n";
            debug(
            //for(auto five_tuple : five_tuple_history)
            //{
            //    for(auto pkt_idx : five_tuple.second.second)
            //    {
            //        std::cout << "Index : " << pkt_idx << " Five Tuple: " << five_tuple.first << "\n";
            //    }
            //}
            )
            std::cout << "------ End Content of the five tuple history ------\n";

        }

        virtual FiveTuple select_replacement_victim(const FiveTuple& five_tuple, size_t timestamp) override
        {
            // Expects: five_tuple is not in cache; cache is full; calling this method is the only way to remove something from cache and five_tuple will be inserted in cache after the call.

            auto find_next_usage = [timestamp](fivetuple_history_t::mapped_type& history) {
                size_t never_seen_again = std::numeric_limits<size_t>::max();
                while (history.first < history.second.size() && history.second[history.first] <= timestamp)
                    history.first++;
                return history.first < history.second.size()
                    ? history.second[history.first]
                    : never_seen_again;
            };
            auto push_next_usage_of = [this, &find_next_usage](const FiveTuple& key) {
                auto it = five_tuple_history.find(key);
                if (it == five_tuple_history.end())
                    std::cout << "Error, key not found in history\n";
                    
                replacement_queue.emplace(find_next_usage(it->second), it);
            };

            // Fill queue on first call, the cache is full (as stated in Expects above).
            if (replacement_queue.empty())
                for (const auto& [key, value] : this->lookup_table_)
                    push_next_usage_of(key);
            else
            {
                // Update the next usages for current time.
                typename replacement_queue_t::iterator bottom;
                while ((bottom = replacement_queue_min_iterator())->next_usage_time <= timestamp)
                    replacement_queue.update(bottom, {find_next_usage(bottom->fivetuple_history_entry->second), bottom->fivetuple_history_entry});
            }
            // The victim is the element used farthest in the future, thus the top of the queue.
            const ReplacementQueueElement& top = replacement_queue.top();
            if (top.next_usage_time <= timestamp)
                std::cout << "Error, next usage is before current time\n";
            const FiveTuple victim_fivetuple =  top.fivetuple_history_entry->first;
            if (this->lookup_table_.find(victim_fivetuple) == this->lookup_table_.end())
                std::cout << "Error, victim is not in cache\n";
            replacement_queue.pop();
            // Add the new five tuple.
            push_next_usage_of(five_tuple);
            return victim_fivetuple;
        }

    private:
        // priority_deque provides maximum() and minimum() methods returing values but not iterators.  update() requires an iterator.
        typename replacement_queue_t::iterator replacement_queue_min_iterator() {
            return replacement_queue.begin(); // see minimum()
        }

        fivetuple_history_t five_tuple_history;
        const std::string pcap_pre_file_name;
        replacement_queue_t replacement_queue;
};

template<typename lookup_table_t, typename  stats_table_t, typename promo_stats_table_t>
class LFUPolicy final: public Policer<lookup_table_t,stats_table_t,promo_stats_table_t>
{
    private:
        const CounterType counter_type_;

    public:
        using policer_t =  Policer<lookup_table_t, stats_table_t,promo_stats_table_t>;

        LFUPolicy(const lookup_table_t& lookup_table,
                        stats_table_t& stats_table,
                        promo_stats_table_t& promo_stats_table,
                        const CounterType counter_type) :
                        policer_t(lookup_table, stats_table, promo_stats_table),
                        counter_type_(counter_type)
        {}

        virtual FiveTuple select_replacement_victim(const FiveTuple& five_tuple, size_t pkt_size) override
        {
            // Get the least frequently used entry.
            auto tuple_to_remove = this->stats_table_.get_stats().highest_order();
            auto selected = tuple_to_remove->key;

            this->stats_table_.get_stats().replace(
                tuple_to_remove,
                {
                    five_tuple,
                    ((counter_type_ == CounterType::BYTES) ? pkt_size : 1)
                }
            );

            debug(
            std::cout << "---------------------------\n";
            std::cout << "Lookup table contents\n";
            std::cout << "---------------------------\n";
            for(const auto& [key, _] : this->lookup_table_)
            {
                std::cout <<  key << '\n';
            }
            std::cout << "---------------------------\n";
            std::cout << "Stats table contents\n";
            std::cout << "---------------------------\n";
            for(const auto& [key, val] : this->stats_table_.get_stats())
            {
                std::cout <<  key <<  ", " << val << '\n';
            }
            std::cout << "---------------------------\n";
            std::cout << "Key to remove " << selected << '\n';
            std::cout << "---------------------------\n";
            )

            return selected;

        }

};

template<typename lookup_table_t, typename  stats_table_t, typename promo_stats_table_t>
class OLFUPolicy final: public Policer<lookup_table_t,stats_table_t,promo_stats_table_t>
{
    private:
        const CounterType counter_type_;

    public:
        using policer_t =  Policer<lookup_table_t, stats_table_t,promo_stats_table_t>;

        OLFUPolicy (const lookup_table_t& lookup_table,
                        stats_table_t& stats_table,
                        promo_stats_table_t& promo_stats_table,
                        const CounterType counter_type) :
                        policer_t(lookup_table, stats_table, promo_stats_table),
                        counter_type_(counter_type)
        {}

        virtual FiveTuple select_replacement_victim(const FiveTuple& five_tuple, size_t pkt_size) override
        {
            // Get the least frequently used entry.
            auto tuple_to_remove = this->stats_table_.get_stats().highest_order();
            auto selected = tuple_to_remove->key;
            this->stats_table_.get_stats().replace(
                tuple_to_remove,
                {
                    five_tuple,
                    tuple_to_remove->value + ((counter_type_ == CounterType::BYTES) ? pkt_size : 1)
                }
            );

            debug(
            std::cout << "---------------------------\n";
            std::cout << "Lookup table contents\n";
            std::cout << "---------------------------\n";
            for(const auto& [key, _] : this->lookup_table_)
            {
                std::cout <<  key << '\n';
            }
            std::cout << "---------------------------\n";
            std::cout << "Stats table contents\n";
            std::cout << "---------------------------\n";
            for(const auto& [key, val] : this->stats_table_.get_stats())
            {
                std::cout <<  key <<  ", " << val << '\n';
            }
            std::cout << "---------------------------\n";
            std::cout << "Key to remove " << selected << '\n';
            std::cout << "---------------------------\n";
            )

            return selected;
        }

};

template<typename lookup_table_t, typename  stats_table_t, typename promo_stats_table_t>
class NXUPolicy final: public Policer<lookup_table_t,stats_table_t,promo_stats_table_t>
{

    public:
        using policer_t =  Policer<lookup_table_t, stats_table_t,promo_stats_table_t>;

        NXUPolicy(const lookup_table_t& lookup_table,
                        stats_table_t& stats_table,
                        promo_stats_table_t& promo_stats_table) :
                        policer_t(lookup_table, stats_table, promo_stats_table)
        {}

        virtual FiveTuple select_replacement_victim(const FiveTuple& five_tuple, size_t) override
        {
            // Search for a not recent used entry.
            FiveTuple selected;

            for (const auto& entries : this->lookup_table())
            {
                if(this->stats_table_.get_stats().find(entries.first)
                        == this->stats_table_.get_stats().end())
                {
                    selected = entries.first;
                    break;
                }
            }

            debug(
            std::cout << "---------------------------\n";
            std::cout << "Lookup table contents\n";
            std::cout << "---------------------------\n";
            for(const auto& [key, _] : this->lookup_table_)
            {
                std::cout <<  key << '\n';
            }
            std::cout << "---------------------------\n";
            std::cout << "Stats table contents\n";
            std::cout << "---------------------------\n";
            for(const auto& [key, val] : this->stats_table_.get_stats())
            {
                std::cout <<  key <<  ", " << val << '\n';
            }
            std::cout << "---------------------------\n";
            std::cout << "Key to remove " << selected << '\n';
            std::cout << "---------------------------\n";
            )

            return selected;
        }


};

#endif
