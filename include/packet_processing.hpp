// File: packet_processing.hpp
// Purpose: Packet processing pipeline
// Author: jeferson.silva@polymtl.ca

// STL libs
#include <cstdint>
#include <array>
#include <tuple>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

#ifndef __PKT_PROCESSING__
#define __PKT_PROCESSING__

#include "pkt_common.hpp"
#include "static_hash_map.hpp"
#include "lookup_table.hpp"
#include "stats.hpp"

template<size_t Lookup_Size, typename Lookup_Value, typename Cache_Stats, size_t Sleep_Time = 0>
class PacketProcessing {

    public:
        // Constants
        static constexpr auto LOOKUP_MEM_SIZE = Lookup_Size;

        void process_packet (inter_thread_comm_t& in_comm_pkt, inter_thread_comm_t& out_comm_pkt, const std::size_t read_step, const CacheType cache_type) {

            while (!in_comm_pkt.get_done()) {
                // Wait until a message is pushed to the queue
                discrete_ts = in_comm_pkt.pull_message(packet_timestamp_, read_step);

                // Extract five tuple and packet size
                tuple_size_pair_ = create_five_tuple_from_packet(packet_timestamp_.first);
                num_packets_++;
                std::cout << "Thread ID " << std::this_thread::get_id() << " extracted " << num_packets_ << " five tuples\n";
                std::cout << tuple_size_pair_.first;
   
                // Look table iterator: key + value
                auto lookup_result = lookup_table_.find(tuple_size_pair_.first);
                auto match = lookup_result != lookup_table_.end();

                // Is not held in the cache?
                if (!match) {
                    punt_pkt_to_next_lvl(out_comm_pkt);
                }
                
                // Update cache defined in the derived
                update_cache_stats(match, cache_type);

                if constexpr (Sleep_Time) {
                    std::this_thread::sleep_for(nano_second_t(Sleep_Time));        
                }
            }
        }

        virtual void punt_pkt_to_next_lvl (inter_thread_comm_t& punted_pkt)=0;

        virtual void digest_pkt_to_ctrl (inter_thread_digest_cpu& digest_pkt)=0;

        virtual void update_cache_stats(const bool match, const CacheType cache_type) =0;

        auto& lookup_table () { return lookup_table_; }
        
        auto& get_stats_table () { return stats_table_; }


    protected:
        LookupTable<Lookup_Size, Lookup_Value> lookup_table_;

        // Add stats container
        Cache_Stats stats_table_;
        // Add policy - for stats support
        packet_timestamp_pair_t packet_timestamp_;
        std::size_t num_packets_ = 0;
        std::size_t discrete_ts = 0;
        tuple_pkt_size_pair_t tuple_size_pair_;

}; // PacketProcessing


template<size_t Lookup_Size, typename Lookup_Value, typename Cache_Stats, size_t Sleep_Time = 0>
class CacheL1PacketProcessing final : public PacketProcessing <Lookup_Size, Lookup_Value, Cache_Stats, Sleep_Time> {

    public:
        virtual void punt_pkt_to_next_lvl (inter_thread_comm_t& punted_pkt) override {
            punted_pkt.push_message(this->packet_timestamp_);
        }

        virtual void digest_pkt_to_ctrl (inter_thread_digest_cpu& digest_pkt) override {
            digest_pkt.push_message(this->tuple_size_pair_);
        }
        
        virtual void update_cache_stats(const bool match, const CacheType cache_type) override {
            if (match) {
                if (cache_type == CacheType::LRU) { 
                    this->stats_table_.update_stats(this->tuple_size_pair_.first, this->discrete_ts);
                } else if (cache_type == CacheType::LFU ) {
                    this->stats_table_.update_stats(this->tuple_size_pair_.first, this->tuple_size_pair_.second);
                } else {
                }
            }
        }

};

template<size_t Lookup_Size, typename Lookup_Value, typename Cache_Stats, size_t Sleep_Time = 0>
class CacheL2PacketProcessing final : public PacketProcessing <Lookup_Size, Lookup_Value, Cache_Stats, Sleep_Time> {

    public:
        virtual void punt_pkt_to_next_lvl (inter_thread_comm_t& punted_pkt) override {}
        virtual void digest_pkt_to_ctrl (inter_thread_digest_cpu& digest_pkt) override {}
        virtual void update_cache_stats(const bool match, const CacheType cache_type) override {}

};

#endif // __PKT_PROCESSING__
