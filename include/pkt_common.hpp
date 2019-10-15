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

#ifndef __PKT_COMMON__
#define __PKT_COMMON__

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

std::ostream& operator<<(std::ostream& os, const FiveTuple& five_tuple) {
    os << "{ " << five_tuple.src_addr << ", "
        << five_tuple.dst_addr << ", "
        << (int)five_tuple.protocol << ", "
        << five_tuple.src_port << ", "
        << five_tuple.dst_port << " }\n";
    return os;
}

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

template<typename Message>
struct ThreadCommunication {
    std::queue<Message> mqueue;     // the queue of messages
    std::condition_variable mcond;  // the var iable communicating events
    std::mutex mmutex;              // the locking mechanism
};
#endif // __PKT_COMMON__
