#include <random>
#include <algorithm>

#ifndef __POLICY__
#define __POLICY__


#include "pkt_common.hpp"
#include "controller.hpp"
#include "lookup_table.hpp"
#include "stats.hpp"
// Quels sont les composants communs a une politique de cache.

// Base sur un input (Cache Stats) + Event (Cache Miss), identifie l'entree a ajouter
// (lookup container controller) et l entree a retirer.
// Interface avec le controller.
// Policy doit pouvoir s

// Policier is specialized for a given politic.
// Currently assume that L1 cache enries are updated following a cache miss.

template<typename lookup_table_t, typename  stats_table_t>
class Policier{

    public:

        Policier(const lookup_table_t& lookup_table,
                    stats_table_t& stats_table) :
                    lookup_table_{lookup_table},
                    stats_table_{stats_table}
        {}

        virtual FiveTuple select_replacement_victim(FiveTuple five_tuple, size_t timestamp) =0;

        auto& stats_table () { return stats_table_; }

    protected:
        const lookup_table_t& lookup_table_;
        stats_table_t& stats_table_;

};

template<typename lookup_table_t, typename  stats_table_t>
class RandomPolicy final: public Policier<lookup_table_t,stats_table_t>
{

    public:
        using policer_t = Policier<lookup_table_t, stats_table_t>;

    private:

        // Hide the random generation mechanism
        std::size_t random_number_generation() const
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, lookup_table_t::LOOKUP_MEM_SIZE);
            return dis(gen);
        }

    public:

        RandomPolicy(const lookup_table_t& lookup_table,
                        stats_table_t& stats_table) :
                        policer_t(lookup_table, stats_table)
        {}

        virtual FiveTuple select_replacement_victim(FiveTuple , size_t) override
        {
            // From https://stackoverflow.com/questions/27024269/select-random-element-in-an-unordered-map
            const auto tuple = this->lookup_table_.indexed_iter(random_number_generation()).first;
            debug(std::cout << "Selected " << tuple << " for replacement\n";)
            return tuple;
        }
};

template<typename lookup_table_t, typename  stats_table_t>
class LRUPolicy final: public Policier<lookup_table_t,stats_table_t>
{

    public:
        using policer_t =  Policier<lookup_table_t, stats_table_t>;

        LRUPolicy(const lookup_table_t& lookup_table,
                        stats_table_t& stats_table) :
                        policer_t(lookup_table, stats_table)
        {}

        virtual FiveTuple select_replacement_victim(FiveTuple five_tuple, size_t timestamp) override
        {
            auto value_sort = [](auto a, auto b) { return a.second < b.second; };
            // Get the least recently used entry.
            // TUple + value associe
            // 1. Get stats
            //auto tuple_to_remove = this->stats_table_.front();
            //this->stats_table_.get_stats().data().front() = { five_tuple, tuple_to_remove.second  + pkt_size };
            //std::make_heap(this->stats_table_.get_stats().begin(), this->stats_table_.get_stats().end(), value_sort);

            auto tuple_to_remove = std::min_element(this->stats_table_.get_stats().begin(), this->stats_table_.get_stats().end(), value_sort);
            auto selected = tuple_to_remove->first;
            *tuple_to_remove = { five_tuple, timestamp };

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
            for(const auto& [key, val] : this->stats_table_.get_stats().data())
            {
                std::cout <<  key <<  ", " << val << '\n';
            }
            std::cout << "---------------------------\n";
            std::cout << "Key to remove " << tuple_to_remove->first << '\n';
            std::cout << "---------------------------\n";
            )

            return selected;

        }


};

// Opt Policy uses a ordered list of further seem flows
// These flows need to be evicted
template<typename lookup_table_t, typename  stats_table_t>
class OPTPolicy final : public Policier<lookup_table_t,stats_table_t>
{
    //TODO: validate when we enqueue that the current timestamp is also pushed. Need a global visibility on the timestamp (i.e. a scheduler!)
    public:
        using policer_t =  Policier<lookup_table_t, stats_table_t>;
        using fivetuple_history_t = std::unordered_map<FiveTuple, std::pair<std::size_t, std::vector<std::size_t>>>;

        OPTPolicy(const lookup_table_t& lookup_table,
                        stats_table_t& stats_table, const std::string& file ) :
                        policer_t(lookup_table, stats_table),  file_name{file}
        {
             //build_five_tuple_history();
        }

        void set_current_packet_timestamp(const size_t& timestamp){
            current_packet_timestamp = timestamp;
        }

        void build_five_tuple_history(){
            pcpp::PcapFileReaderDevice reader(file_name.c_str());

            if (!reader.open())
            {
                std::cout << "Error opening the pcap file\n";

            }

            std::size_t pkts = 0;
            pcpp::RawPacket rawPacket;
            while (reader.getNextPacket(rawPacket)) {
                pcpp::Packet parsedPacket(&rawPacket);
                // Create Five Tuple
                const auto& [five_tuple,size] = create_five_tuple_from_packet(parsedPacket);
                // Enqueue FiveTuple
                five_tuple_history[five_tuple].first = 0;
                five_tuple_history[five_tuple].second.push_back(++pkts);
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

        virtual FiveTuple select_replacement_victim(FiveTuple, size_t timestamp) override
        {
            size_t distance_to_farthest_fivetuple {};
            FiveTuple farthest_fivetuple{};
            current_packet_timestamp = timestamp;
            FiveTuple default_entry_removed {};
            bool is_found {false};
            size_t number_keys_found {};
            // Remove any non-re referenced entry.
            // Read the five-tuple key stored in the lookup table
            for(const auto& key_val_tuple : this->lookup_table_)
            {
                const auto& [key, value] = key_val_tuple;
                //default_entry_removed = key;
                //std::cout << "Key Evaluated : " << key << "\n";
                // When is the next reference to this key?

                auto history_it = five_tuple_history.find(key);
                bool is_matched {false};
                if (history_it != five_tuple_history.end())
                {
                    auto& [ini_idx, tuple_vec] = history_it->second;
                    for (auto idx = ini_idx; idx < tuple_vec.size(); ++idx)
                    {
                        auto entry_to_remove = tuple_vec[idx];
                        if (entry_to_remove < current_packet_timestamp)
                        {
                            continue;
                            //history_it->second.erase(entry_to_remove);
                        } else
                        {
                            ini_idx = idx;
                            if((entry_to_remove - current_packet_timestamp) >= distance_to_farthest_fivetuple)
                            {
                                distance_to_farthest_fivetuple = entry_to_remove - current_packet_timestamp;
                                farthest_fivetuple = key;
                                is_found =  true;
                            }
                            number_keys_found++;
                            is_matched = true;
                            // Exit at the first reference.
                            break;
                        }
                    }
                }
                // Key not found ! Can be removed !
                if(!is_matched){
                    default_entry_removed = key;
                    debug(std::cout<< "Default Entry to be removed: " << default_entry_removed << "\n";)
                }
            }
            if (is_found && number_keys_found > 1)
            {
                debug(
                std::cout << "---------------------------\n";
                std::cout << "Lookup table contents\n";
                std::cout << "---------------------------\n";
                for(const auto& [key, _] : this->lookup_table_)
                {
                    std::cout <<  key << '\n';
                }
                std::cout << "---------------------------\n";
                std::cout << "Key to remove " << farthest_fivetuple << '\n';
                std::cout << "---------------------------\n";
                )
                return farthest_fivetuple;

            } else
            {
                debug(std::cout <<  "Return default entry \n";)
                return default_entry_removed;
            }

        }


    private:
        size_t current_packet_timestamp;
        fivetuple_history_t five_tuple_history;
        const std::string file_name;

};

template<typename lookup_table_t, typename  stats_table_t>
class LFUPolicy final: public Policier<lookup_table_t,stats_table_t>
{
    private:
        const CounterType counter_type_;

    public:
        using policer_t =  Policier<lookup_table_t, stats_table_t>;

        LFUPolicy(const lookup_table_t& lookup_table,
                        stats_table_t& stats_table,
                        const CounterType counter_type) :
                        policer_t(lookup_table, stats_table),
                        counter_type_(counter_type)
        {}

        virtual FiveTuple select_replacement_victim(FiveTuple five_tuple, size_t pkt_size) override
        {
            auto value_sort = [](auto a, auto b) { return a.second < b.second; };
            // Get the least recently used entry.
            // TUple + value associe
            // 1. Get stats
            //auto tuple_to_remove = this->stats_table_.front();
            //this->stats_table_.get_stats().data().front() = { five_tuple, tuple_to_remove.second  + pkt_size };
            //std::make_heap(this->stats_table_.get_stats().begin(), this->stats_table_.get_stats().end(), value_sort);

            auto tuple_to_remove = std::min_element(this->stats_table_.get_stats().begin(), this->stats_table_.get_stats().end(), value_sort);
            auto selected = tuple_to_remove->first;
            *tuple_to_remove = { five_tuple, ((counter_type_ == CounterType::BYTES) ? pkt_size : 1) };

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
            for(const auto& [key, val] : this->stats_table_.get_stats().data())
            {
                std::cout <<  key <<  ", " << val << '\n';
            }
            std::cout << "---------------------------\n";
            std::cout << "Key to remove " << tuple_to_remove->first << '\n';
            std::cout << "---------------------------\n";
            )

            return selected;

        }

};


template<typename lookup_table_t, typename  stats_table_t>
class LFUModifPolicy final: public Policier<lookup_table_t,stats_table_t>
{
    private:
        const CounterType counter_type_;

    public:
        using policer_t =  Policier<lookup_table_t, stats_table_t>;

        LFUModifPolicy(const lookup_table_t& lookup_table,
                        stats_table_t& stats_table,
                        const CounterType counter_type) :
                        policer_t(lookup_table, stats_table),
                        counter_type_(counter_type)
        {}

        virtual FiveTuple select_replacement_victim(FiveTuple five_tuple, size_t pkt_size) override
        {
            auto value_sort = [](auto a, auto b) { return a.second < b.second; };
            // Get the least recently used entry.
            // TUple + value associe
            // 1. Get stats
            //auto tuple_to_remove = this->stats_table_.front();
            //this->stats_table_.get_stats().data().front() = { five_tuple, tuple_to_remove.second  + pkt_size };
            //std::make_heap(this->stats_table_.get_stats().begin(), this->stats_table_.get_stats().end(), value_sort);

            auto tuple_to_remove = std::min_element(this->stats_table_.get_stats().begin(), this->stats_table_.get_stats().end(), value_sort);
            auto selected = tuple_to_remove->first;
            *tuple_to_remove = { five_tuple, tuple_to_remove->second  +
                                 ((counter_type_ == CounterType::BYTES) ? pkt_size : 1) };
#if 0
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
            for(const auto& [key, val] : this->stats_table_.get_stats().data())
            {
                std::cout <<  key <<  ", " << val << '\n';
            }
            std::cout << "---------------------------\n";
            std::cout << "Key to remove " << tuple_to_remove->first << '\n';
            std::cout << "---------------------------\n";
            )
#endif
            return selected;

        }

};

template<typename lookup_table_t, typename  stats_table_t>
class MFUPolicy final: public Policier<lookup_table_t,stats_table_t>
{
    private:
        const CounterType counter_type_;

    public:
        using policer_t =  Policier<lookup_table_t, stats_table_t>;

        MFUPolicy(const lookup_table_t& lookup_table,
                        stats_table_t& stats_table,
                        const CounterType counter_type) :
                        policer_t(lookup_table, stats_table),
                        counter_type_(counter_type)
        {}

        virtual FiveTuple select_replacement_victim(FiveTuple, size_t) override
        {
            auto value_sort = [](auto a, auto b) { return a.second > b.second; };
            // Get the least recently used entry.
            // TUple + value associe
            // 1. Get stats
            //auto tuple_to_remove = this->stats_table_.front();
            //this->stats_table_.get_stats().data().front() = { five_tuple, tuple_to_remove.second  + pkt_size };
            //std::make_heap(this->stats_table_.get_stats().begin(), this->stats_table_.get_stats().end(), value_sort);

            auto& tuple_to_remove = this->stats_table_.get_stats().front();
            auto selected = tuple_to_remove.first;
            tuple_to_remove.second = 0;
            this->stats_table_.get_stats().sort(value_sort);
#if 0
            debug(
            std::cout << "---------------------------\n";
            std::cout << "Lookup table contents\n";
            std::cout << "---------------------------\n";
            for(const auto& [key, _] : this->lookup_table_)
            {
                std::cout <<  key << '\n';
            }
            )
#endif
            std::cout << "---------------------------\n";
            std::cout << "Stats table contents\n";
            std::cout << "---------------------------\n";
            for(const auto& [key, val] : this->stats_table_.get_stats().data())
            {
                std::cout <<  key <<  ", " << val << '\n';
            }
            std::cout << "---------------------------\n";
            std::cout << "Key to remove " << tuple_to_remove.first << '\n';
            std::cout << "---------------------------\n";
            return selected;

        }

};

#endif

