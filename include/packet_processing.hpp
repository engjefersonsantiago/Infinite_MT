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

template<size_t Lookup_Size, typename Lookup_Value>
class PacketProcessing {

    public:
        // Constants
        static constexpr auto LOOKUP_MEM_SIZE = Lookup_Size;

        auto lookup (const FiveTuple& five_tuple) const {
            return lookup_table_.find(five_tuple) != lookup_table_.end();
        }

        void process_packet (inter_thread_comm_t& in_comm_pkt, [[maybe_unused]] inter_thread_comm_t& out_comm_pkt) {

            while (!in_comm_pkt.get_done()) {
                // Wait until a message is pushed to the queue
                in_comm_pkt.pull_message(packet_timestamp);
               
                // Extract five tuple and packet size 
                auto [five_tuple, pkt_size] = create_five_tuple_from_packet(packet_timestamp.first);
                num_packets_++;
                std::cout << "Thread ID " << std::this_thread::get_id() << " extracted " << num_packets_ << " five tuples\n";
                std::cout << five_tuple;

                if (!lookup(five_tuple)) {
                    punt_pkt(out_comm_pkt);
                }
            }
        }

        auto add_key (const FiveTuple& five_tuple, const Lookup_Value& value) {
            if (capacity_ < LOOKUP_MEM_SIZE) {
                capacity_++;
                lookup_table_.insert(five_tuple, value);
                return true;
            } else {
                return false;
            }
        }

        auto delete_key (const FiveTuple& five_tuple) {
            if (lookup(five_tuple)) {
                capacity_--;
                lookup_table_.erase(five_tuple);
                return true;
            } else {
                return false;
            }
        } 

        auto replace_key (const FiveTuple& old_tuple, const FiveTuple& new_tuple, const Lookup_Value& new_value) {
            if (delete_key(old_tuple)) {
                return add_key(new_tuple, new_value);
            } else {
                return false;
            }
        } 

        virtual void punt_pkt (inter_thread_comm_t& punted_pkt) const =0;

    protected:
        std::unordered_map<FiveTuple, Lookup_Value> lookup_table_;
        packet_timestamp_pair_t packet_timestamp_;
        size_t capacity_ = 0;
        size_t num_packets_ = 0;
        packet_timestamp_pair_t packet_timestamp;

}; // PacketProcessing


template<size_t Lookup_Size, typename Lookup_Value>
class CacheL1PacketProcessing final : public PacketProcessing <Lookup_Size, Lookup_Value> {

    public:
        virtual void punt_pkt (inter_thread_comm_t& punted_pkt) const override {
            punted_pkt.push_message(this->packet_timestamp);
        }

};

#endif // __PKT_PROCESSING__ 
