// File: main_pipe.hpp
// Purpose: Main processing pipeline
// Author: jeferson.silva@polymtl.ca

// STL libs
#include <cstdint>
#include <array>
#include <tuple>
#include <thread>
#include <mutex>
#include <condition_variable>

#ifndef __MAIN_PIPE__
#define __MAIN_PIPE__

// Pcap++
#include <PcapFileDevice.h>
#include <IPv4Layer.h>
#include <IPv6Layer.h>
#include <TcpLayer.h>
#include <UdpLayer.h>
#include <Packet.h>

#include "pkt_common.hpp"
#include "static_hash_map.hpp"

// TODO: Remove CommPacket
template<size_t Lookup_Size, typename Lookup_Value, typename CommPacket>
class MainPipe {

    public: 
        // TODO: Move to pkt_common as a standalone function
        auto create_five_tuple_from_packet (pcpp::Packet& parsedPacket) {
            // 5 tuple
            FiveTuple five_tuple {};
            size_t pkt_size = 0;
            
            // verify the packet is L4
            if (parsedPacket.isPacketOfType(pcpp::TCP) || parsedPacket.isPacketOfType(pcpp::UDP)) {

                // Extract L4 fields to build the 5 tuple
                if (parsedPacket.isPacketOfType(pcpp::TCP)) {
                    auto tcp_layer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();
                    five_tuple.src_port = tcp_layer->getTcpHeader()->portSrc;
                    five_tuple.dst_port = tcp_layer->getTcpHeader()->portDst;
                } else {
                    auto udp_layer = parsedPacket.getLayerOfType<pcpp::UdpLayer>();
                    five_tuple.src_port = udp_layer->getUdpHeader()->portSrc;
                    five_tuple.dst_port = udp_layer->getUdpHeader()->portDst;
                }

                // Extract L3 fields to build the remaining of the 5 tuple
                if (parsedPacket.isPacketOfType(pcpp::IPv6)) {
                    auto ipv6_layer = parsedPacket.getLayerOfType<pcpp::IPv6Layer>();
                    five_tuple.src_addr = ipv6_layer->getSrcIpAddress().toString();
                    five_tuple.dst_addr = ipv6_layer->getDstIpAddress().toString();
                    five_tuple.protocol = ipv6_layer->getIPv6Header()->nextHeader;
                    // Because the IPv6 PayloadLength does not take into account the IPv6 header itself, we
                    // add 40 bytes, the IPV6 header size, to measure the same amount of information over an IPv4 header.
                    pkt_size = ipv6_layer->getIPv6Header()->payloadLength + 40; // create a constant here
                } else {
                    auto ipv4_layer = parsedPacket.getLayerOfType<pcpp::IPv4Layer>();
                    five_tuple.src_addr = ipv4_layer->getSrcIpAddress().toString();
                    five_tuple.dst_addr = ipv4_layer->getDstIpAddress().toString();
                    five_tuple.protocol = ipv4_layer->getIPv4Header()->protocol;
                    pkt_size = ipv4_layer->getIPv4Header()->totalLength;
                }
            }
            return std::make_pair(five_tuple, pkt_size);
        }

        void process_packet (CommPacket& comm_pkt) {

            while (true) {
                std::unique_lock<std::mutex> lck{comm_pkt.mmutex}; // acquire mmutex
                comm_pkt.mcond.wait(lck);
                /* do nothing */
                // release lck and wait;
                
                // re-acquire lck upon wakeup
                auto [packet, timestamp] = comm_pkt.mqueue.front(); // get the message
                comm_pkt.mqueue.pop();
                lck.unlock(); //release lck
                // ... process m ...
                
                auto [five_tuple, pkt_size] = create_five_tuple_from_packet(packet);
            }
        }

    public:
        StaticHashMap<Lookup_Size, FiveTuple, Lookup_Value> lookup_table_;

}; // MainPipe

#endif
