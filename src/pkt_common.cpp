#include "pkt_common.hpp"

std::ostream& operator<<(std::ostream& os, const FiveTuple& five_tuple)
{
    os << "{ " << five_tuple.src_addr << ", "
        << five_tuple.dst_addr << ", "
        << (int)five_tuple.protocol << ", "
        << five_tuple.src_port << ", "
        << five_tuple.dst_port << " }";
    return os;
}

std::size_t hash_value(const FiveTuple& tuple)
{
    return std::hash<FiveTuple>()(tuple);
}

// Helper functions
tuple_pkt_size_pair_t create_five_tuple_from_packet (/*const*/ pcpp::Packet& parsedPacket)
{
    // 5 tuple
    FiveTuple five_tuple{};
    size_t pkt_size = 0;


    // verify the packet is L4
    if (parsedPacket.isPacketOfType(pcpp::TCP) || parsedPacket.isPacketOfType(pcpp::UDP))
    {

        // Extract L4 fields to build the 5 tuple
        if (parsedPacket.isPacketOfType(pcpp::TCP))
        {
            auto tcp_layer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();
            five_tuple.src_port = htons(tcp_layer->getTcpHeader()->portSrc);
            five_tuple.dst_port = htons(tcp_layer->getTcpHeader()->portDst);
        } else
        {
            auto udp_layer = parsedPacket.getLayerOfType<pcpp::UdpLayer>();
            five_tuple.src_port = htons(udp_layer->getUdpHeader()->portSrc);
            five_tuple.dst_port = htons(udp_layer->getUdpHeader()->portDst);
        }

        // Extract L3 fields to build the remaining of the 5 tuple
        //NOTE: Construction of a pcpp::IPV4Address or IPV6Address (by getSrcIpAddress and getDstIpAddress) will call inet_ntop, which is slow, we thus access the header directly.
        if (parsedPacket.isPacketOfType(pcpp::IPv6))
        {
            auto ipv6_layer = parsedPacket.getLayerOfType<pcpp::IPv6Layer>();
            auto ip_header = ipv6_layer->getIPv6Header();
            five_tuple.src_addr = ip_header->ipSrc;
            five_tuple.dst_addr = ip_header->ipDst;
            five_tuple.protocol = ipv6_layer->getIPv6Header()->nextHeader;
            // Because the IPv6 PayloadLength does not take into account the IPv6 header itself, we
            // add 40 bytes, the IPV6 header size, to measure the same amount of information over an IPv4 header.
            pkt_size = htons(ipv6_layer->getIPv6Header()->payloadLength) + 40; // create a constant here
        } else
        {
            auto ipv4_layer = parsedPacket.getLayerOfType<pcpp::IPv4Layer>();
            auto ip_header = ipv4_layer->getIPv4Header();
            five_tuple.src_addr = IpAddress::fromIPv4_netByteOrder(ip_header->ipSrc);
            five_tuple.dst_addr = IpAddress::fromIPv4_netByteOrder(ip_header->ipDst);
            five_tuple.protocol = ipv4_layer->getIPv4Header()->protocol;
            pkt_size = htons(ipv4_layer->getIPv4Header()->totalLength);
        }
    }
    return std::make_pair(five_tuple, pkt_size);
}

std::unordered_set<FiveTuple> filter_unique_tuples_from_trace (const std::string& pcap_file) {
    std::unordered_set<FiveTuple> five_tuples;
    pcpp::PcapFileReaderDevice reader(pcap_file.c_str());

    if (!reader.open()) { std::cout << "Error opening the pcap file\n"; }

    pcpp::RawPacket rawPacket;
    while (reader.getNextPacket(rawPacket)) {
        // Push Packet
        pcpp::Packet packet(&rawPacket);
        const auto& [tuple, _] = create_five_tuple_from_packet(packet);
        if (five_tuples.find(tuple) == five_tuples.end()) {
            five_tuples.insert(tuple);
        }
    }
    return five_tuples;
}

