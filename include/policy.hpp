#include <random>

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
                    const stats_table_t& stats_table) :
                    lookup_table_{lookup_table},
                    stats_table_{stats_table}
        {}

        virtual FiveTuple select_replacement_victim() const =0;

    protected:
        const lookup_table_t& lookup_table_;
        const stats_table_t& stats_table_;

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
                        const stats_table_t& stats_table) :
                        policer_t(lookup_table, stats_table)
        {}

        virtual FiveTuple select_replacement_victim() const override
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
                        const stats_table_t& stats_table) :
                        policer_t(lookup_table, stats_table)
        {}

        virtual FiveTuple select_replacement_victim() const override
        {
            // Get the least recently used entry.
            // TUple + value associe
            // 1. Get stats
            return this->stats_table_.back().first;
        }

};

// Opt Policy uses a ordered list of further seem flows
// These flows need to be evicted
template<typename lookup_table_t, typename  stats_table_t>
//using OPTPolicy = LRUPolicy<lookup_table_t, stats_table_t>;
class OPTPolicy final : public Policier<lookup_table_t,stats_table_t>
{
    //TODO: validate when we enqueue that the current timestamp is also pushed. Need a global visibility on the timestamp (i.e. a scheduler!) 
    public:
        using policer_t =  Policier<lookup_table_t, stats_table_t>;
        using fivetuple_history_t = std::vector<FiveTuple>;

        OPTPolicy(const lookup_table_t& lookup_table,
                        const stats_table_t& stats_table, const std::string& file ) :
                        policer_t(lookup_table, stats_table),  file_name{file}
        {
             build_five_tuple_history();
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


            pcpp::RawPacket rawPacket;
            while (reader.getNextPacket(rawPacket)) {
                pcpp::Packet parsedPacket(&rawPacket);
                // Create Five Tuple 
                const auto& [five_tuple,size] = create_five_tuple_from_packet(parsedPacket);
                // Enqueue FiveTuple
                five_tuple_history.push_back(five_tuple);



            }
        }

        virtual FiveTuple select_replacement_victim() const override
        {
            size_t distance_to_farthest_fivetuple {};
            FiveTuple farthest_fivetuple{};
            // Read the five-tuple key stored in the lookup table
            for( auto  key_val_tuple : this->lookup_table_ ){
                const auto& [key,value] = key_val_tuple;

                // When is the next reference to this key?
                for(auto index_it = current_packet_timestamp; index_it < five_tuple_history.size(); index_it++ ){
                    if(five_tuple_history[index_it] == key){
                        if((index_it - current_packet_timestamp) > distance_to_farthest_fivetuple){
                            distance_to_farthest_fivetuple = index_it - current_packet_timestamp;
                            farthest_fivetuple = key;
                        }
                    break;
                    }

                }

            }
        return  farthest_fivetuple;   

        }


    private:
        size_t current_packet_timestamp;
        fivetuple_history_t five_tuple_history;
        const std::string file_name;

};

template<typename lookup_table_t, typename  stats_table_t>
class LFUPolicy final: public Policier<lookup_table_t,stats_table_t>
{

    public:
        using policer_t =  Policier<lookup_table_t, stats_table_t>;

        LFUPolicy(const lookup_table_t& lookup_table,
                        const stats_table_t& stats_table) :
                        policer_t(lookup_table, stats_table)
        {}

        virtual FiveTuple select_replacement_victim() const override
        {
            // Get the least recently used entry.
            // TUple + value associe
            // 1. Get stats
            return this->stats_table_.back().first;
        }

};
#endif
