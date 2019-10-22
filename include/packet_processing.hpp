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

template<size_t Lookup_Size, typename Lookup_Value, size_t Sleep_Time = 0>
class PacketProcessing {

    public:
        // Constants
        static constexpr auto LOOKUP_MEM_SIZE = Lookup_Size;

        void process_packet (inter_thread_comm_t& in_comm_pkt, inter_thread_comm_t& out_comm_pkt) {

            while (!in_comm_pkt.get_done()) {
                // Wait until a message is pushed to the queue
                in_comm_pkt.pull_message(packet_timestamp_);

                // Extract five tuple and packet size
                tuple_size_pair_ = create_five_tuple_from_packet(packet_timestamp_.first);
                num_packets_++;
                std::cout << "Thread ID " << std::this_thread::get_id() << " extracted " << num_packets_ << " five tuples\n";
                std::cout << tuple_size_pair_.first;

                if (!lookup_table_.lookup(tuple_size_pair_.first)) {
                    punt_pkt_to_next_lvl(out_comm_pkt);
                }

                if constexpr (Sleep_Time) {
                    std::this_thread::sleep_for(nano_second_t(Sleep_Time));        
                }
            }
        }

        virtual void punt_pkt_to_next_lvl (inter_thread_comm_t& punted_pkt)=0;

        virtual void digest_pkt_to_ctrl ( inter_thread_digest_cpu& digest_pkt)=0;

        auto& lookup_table () { return lookup_table_; }

    protected:
        LookupTable<Lookup_Size, Lookup_Value> lookup_table_;
        packet_timestamp_pair_t packet_timestamp_;
        size_t num_packets_ = 0;
        tuple_pkt_size_pair_t tuple_size_pair_;

}; // PacketProcessing


template<size_t Lookup_Size, typename Lookup_Value, size_t Sleep_Time = 0>
class CacheL1PacketProcessing final : public PacketProcessing <Lookup_Size, Lookup_Value, Sleep_Time> {

    public:
        virtual void punt_pkt_to_next_lvl ( inter_thread_comm_t& punted_pkt) override {
            punted_pkt.push_message(this->packet_timestamp_);
        }

        virtual void digest_pkt_to_ctrl ( inter_thread_digest_cpu& digest_pkt) override {
            digest_pkt.push_message(this->tuple_size_pair_);
        }

};

template<size_t Lookup_Size, typename Lookup_Value, size_t Sleep_Time = 0>
class CacheL2PacketProcessing final : public PacketProcessing <Lookup_Size, Lookup_Value, Sleep_Time> {

    public:
        virtual void punt_pkt_to_next_lvl (inter_thread_comm_t& punted_pkt) override {}
        virtual void digest_pkt_to_ctrl ( inter_thread_digest_cpu& digest_pkt) override {}

};

#endif // __PKT_PROCESSING__
