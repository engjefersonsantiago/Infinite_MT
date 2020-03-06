// File: pkt_common.hpp
// Purpose: Header for common packet data structures
// Author: jeferson.silva@polymtl.ca

// STL libs
#include <iostream>
#include <cstdint>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <queue>
#include <tuple>
#include <memory>
#include <unordered_set>
#include <atomic>
#include <random>
#include <algorithm>

#include <boost/circular_buffer.hpp>
#include <arpa/inet.h>


#ifndef __PKT_COMMON__
#define __PKT_COMMON__

// Pcap++
#include <PcapFileDevice.h>
#include <IPv4Layer.h>
#include <IPv6Layer.h>
#include <TcpLayer.h>
#include <UdpLayer.h>
#include <Packet.h>

#include "pipeline_params.hpp"

//#define DEBUG
#ifdef DEBUG
	#define debug(...) __VA_ARGS__
#else
	#define debug(...)
#endif

using nano_second_t = std::chrono::nanoseconds;
using second_t = std::chrono::seconds;

inline void printIpV6Part(std::ostream& os, uint64_t value, bool isLastPart) {
    static constexpr unsigned nDigits = 32;
    static constexpr unsigned digitsGrouping = 4;
    for (unsigned i = 0; i < nDigits; ++i) {
        char digit = char(value >> (64-4));
        digit += (digit < 10) ? '0' : 'A';
        os << digit;
        value <<= 4;
        if (isLastPart && value == 0) {
            os << "::";
            return;
        }
        else if (i%digitsGrouping == digitsGrouping-1)
            os << ":";
    }
}

inline uint64_t bytes_to_uint64(const uint8_t* bytes)
{
    uint64_t result = 0;
    for (unsigned i = 0; i < 8; ++i)
        result |= uint64_t{bytes[i]} << (64 - (i+1)*8);
    return result;
}

struct IpAddress {
    uint64_t high, low;

    IpAddress() = default;
    IpAddress(uint64_t high_, uint64_t low_) : high(high_), low(low_) {}
    IpAddress(const pcpp::IPv6Address& ipAddress) { *this = ipAddress; }
    IpAddress(const pcpp::IPv4Address& ipAddress) { *this = ipAddress; }

    IpAddress& operator=(const uint8_t (&ipAddress)[16]) {
        return *this = IpAddress{ bytes_to_uint64(ipAddress + 0), bytes_to_uint64(ipAddress + 8) };
    }
    IpAddress& operator=(/*const*/ pcpp::IPv6Address& ipAddress) {
        return *this = ipAddress.toIn6Addr()->s6_addr;
    }
    IpAddress& operator=(const pcpp::IPv4Address& ipAddress) {
        return *this = fromIPv4_netByteOrder(ipAddress.toInt());
    }

    bool operator==(const IpAddress& other) const { return high == other.high && low == other.low; }
    friend std::ostream& operator<<(std::ostream& os, const IpAddress& ipAddress) {
        if (ipAddress.isIpV4())
            os << ((ipAddress.low>>24)&0xFF) << '.' << ((ipAddress.low>>16)&0xFF) << '.' << ((ipAddress.low>>8)&0xFF) << '.' << ((ipAddress.low>>0)&0xFF);
        else {
            printIpV6Part(os, ipAddress.high, ipAddress.low == 0);
            if (ipAddress.low != 0)
                printIpV6Part(os, ipAddress.low, true);
        }
        return os;
    }

    bool isIpV4() const {
        // IPv4 mapped addresses range in IPv6
        return high == 0 && (low & 0xFFFFFFFF00000000ULL) == 0x0000FFFF00000000ULL;
    }

    // Not a constructor as uint32_t is too common and could be in another byte order.
    static IpAddress fromIPv4_netByteOrder(uint32_t address_in_netByteOrder) {
        return IpAddress{ 0, 0x0000FFFF00000000ULL | ntohl(address_in_netByteOrder) };
    }
};
// FiveTuple struct
struct FiveTuple {
    IpAddress src_addr;
    IpAddress dst_addr;
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

template <> struct hash<IpAddress> {
    size_t operator()(const IpAddress& ipAddress) const {
        return std::hash<std::uint64_t>()(ipAddress.high)
                ^ std::hash<std::uint64_t>()(ipAddress.low);
    }
};

template <> struct hash<FiveTuple> {
    size_t operator()(const FiveTuple& tuple) const {
        return std::hash<IpAddress>()(tuple.src_addr)
                ^ std::hash<IpAddress>()(tuple.dst_addr)
                ^ std::hash<uint8_t>()(tuple.protocol)
                ^ std::hash<uint16_t>()(tuple.src_port)
                ^ std::hash<uint16_t>()(tuple.dst_port);
    }
}; // hash<FiveTuple>

} // std

// Hash for Boost
std::size_t hash_value(const FiveTuple& tuple);

template<typename Message, typename Queue = std::queue<Message>, std::size_t Size = 1>
struct ThreadCommunication {
    using circ_buffer_t = boost::circular_buffer<Message>;
    static constexpr auto is_circ_buffer = std::is_same_v<Queue, circ_buffer_t>;
    Queue mqueue;     // the queue of messages
    bool done = false;
    bool consumed = false;
    size_t step = 0;
    std::size_t timeout_;

    ThreadCommunication () : timeout_(1){
        if constexpr (is_circ_buffer)
            mqueue.set_capacity(Size);
    }

    ThreadCommunication (const std::size_t timeout) : timeout_(timeout) {
        if constexpr (is_circ_buffer)
            mqueue.set_capacity(Size);
    }

    void set_done() {
        done = true;
    }

    bool get_done() {
        return done;
    }

    void push_message_two_notify (Message&& message) {
        if constexpr (is_circ_buffer)
            mqueue.push_back(std::move(message));
        else
            mqueue.push(std::move(message));
        ++step;
        consumed = true;
    }

    void push_message_two_notify (const Message& message) {
        if constexpr (is_circ_buffer)
            mqueue.push_back(message);
        else
            mqueue.push(message);
        ++step;
        consumed = true;
    }

    void push_message (Message&& message) {
        if constexpr (is_circ_buffer)
            mqueue.push_back(std::move(message));
        else
            mqueue.push(std::move(message));
        ++step;
        consumed = true;
    }

    void push_message (const Message& message) {
        if constexpr (is_circ_buffer)
            mqueue.push_back(message);
        else
            mqueue.push(message);
        ++step;
        //std::cout << "Push step " << step.load() << '\n';
        consumed = true;
    }

    auto pull_message (Message& message, const std::size_t read_step){
        //if (read_step == 1) { mcond.notify_one(); }
        if (mqueue.empty() || done || step%read_step != 0)
        {
            return std::make_pair(true, std::size_t(0));
        }
        consumed = false;
        message = std::move(mqueue.front());
        if constexpr (is_circ_buffer)
            mqueue.pop_front();
        else
            mqueue.pop();
        return std::make_pair(false, step);
    }
};


// Types
using packet_timestamp_pair_t = std::pair<pcpp::Packet, double>; //TODO: should we eliminate this?
using tuple_pkt_size_pair_t = std::pair<FiveTuple, size_t>;

// The packet information needed for processing.  This info can be stored in a binary file instead of reparsing PCAP at each execution.
struct ParsedPacket {
    FiveTuple five_tuple;
    size_t pkt_size;
    double timestamp;

    void write_to(std::ostream& binary_os) const { binary_os.write(reinterpret_cast<const char*>(this), sizeof(*this)); }
};

using inter_thread_comm_t = ThreadCommunication<ParsedPacket>;
using inter_thread_digest_cpu = ThreadCommunication<tuple_pkt_size_pair_t, boost::circular_buffer<tuple_pkt_size_pair_t>, CACHE_HOST_PROC_SLOWDOWN_FACTOR>; //TODO: Could use ParsedPacket instead of pairs.


// Helper functions
tuple_pkt_size_pair_t create_five_tuple_from_packet (/*const*/ pcpp::Packet& parsedPacket);
std::unordered_set<FiveTuple> filter_unique_tuples_from_trace (const std::string& pcap_file);

#endif // __PKT_COMMON__
