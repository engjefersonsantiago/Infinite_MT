// File: pkt_common.hpp
// Purpose: Header for common packet data structures
// Author: jeferson.silva@polymtl.ca

// STL libs
#include <iostream>
#include <cstdint>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <tuple>
#include <memory>
#include <boost/circular_buffer.hpp>

#ifndef __PKT_COMMON__
#define __PKT_COMMON__

// Pcap++
#include <PcapFileDevice.h>
#include <IPv4Layer.h>
#include <IPv6Layer.h>
#include <TcpLayer.h>
#include <UdpLayer.h>
#include <Packet.h>

// FiveTuple struct
struct FiveTuple {
    std::string src_addr;   // Evaluate changing to bitset
    std::string dst_addr;
    uint8_t protocol;
    uint16_t src_port;
    uint16_t dst_port;

    bool operator==(const FiveTuple& other) const {
        return this->src_addr  == other.src_addr
            && this->dst_addr  == other.dst_addr
            && this->protocol  == other.protocol
            && this->src_port  == other.src_port
            && this->dst_port  == other.dst_port;
    }

    friend std::ostream& operator<<(std::ostream& os, const FiveTuple& five_tuple);
}; // FiveTuple

std::ostream& operator<<(std::ostream& os, const FiveTuple& five_tuple);

// Custom FiveTuple hash inserted into STD
namespace std {

template <> struct hash<FiveTuple> {
    size_t operator()(const FiveTuple& tuple) const {
        return std::hash<std::string>()(tuple.src_addr)
                ^ std::hash<std::string>()(tuple.dst_addr)
                ^ std::hash<uint8_t>()(tuple.protocol)
                ^ std::hash<uint16_t>()(tuple.src_port)
                ^ std::hash<uint16_t>()(tuple.dst_port);
    }
}; // hash<FiveTuple>

} // std

template<typename Message, typename Queue = std::queue<Message>, std::size_t Size = 1>
struct ThreadCommunication {
    Queue mqueue;     // the queue of messages
    std::condition_variable mcond;  // the variable communicating events
    std::mutex mmutex;              // the locking mechanism
    bool done = false;
    std::size_t step = 0;

    ThreadCommunication () {
        if constexpr (std::is_same_v<Queue, boost::circular_buffer<Message>>)
            mqueue(Size);
    }

    void set_done() {
        std::unique_lock<std::mutex> lck {mmutex};
        done = true;
    }
    
    bool get_done() {
        std::unique_lock<std::mutex> lck {mmutex};
        return done/* && mqueue.empty()*/;
    }

    void push_message (Message&& message) {
        std::unique_lock<std::mutex> lck {mmutex};
        mqueue.push(std::move(message));
        ++step;
        mcond.notify_one();
    }

    void push_message (Message& message) {
        std::unique_lock<std::mutex> lck {mmutex};
        mqueue.push(std::move(message));
        ++step;
        mcond.notify_one();
    }
    auto pull_message (Message& message, const std::size_t read_step){ 
        std::unique_lock<std::mutex> lck {mmutex};
        mcond.wait(lck, [=](){ return !mqueue.empty() && (step%read_step == 0); });
        message = std::move(mqueue.front());
        mqueue.pop();
        return step;
    }
};

// Types 
using packet_timestamp_pair_t = std::pair<pcpp::Packet, double>;
using tuple_pkt_size_pair_t = std::pair<FiveTuple, size_t>;
using inter_thread_comm_t = ThreadCommunication<packet_timestamp_pair_t>;
using inter_thread_digest_cpu = ThreadCommunication<tuple_pkt_size_pair_t>;

using nano_second_t = std::chrono::duration<long double, std::nano>;

// Helper functions
tuple_pkt_size_pair_t create_five_tuple_from_packet (pcpp::Packet& parsedPacket);



#endif // __PKT_COMMON__
