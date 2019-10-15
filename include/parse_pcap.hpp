
#ifndef __PARSE_PCAP__
#define __PARSE_PCAP__




// Pcap++
#include <PcapFileDevice.h>
#include <IPv4Layer.h>
#include <IPv6Layer.h>
#include <TcpLayer.h>
#include <UdpLayer.h>
#include <Packet.h>

#include "pkt_common.hpp"




class ParsePackets{
    public: 
    // Type Declaration

    using packet_timestamp_pair_t = std::pair<pcpp::Packet,double>;
    using inter_thread_comm_t = ThreadCommunication<packet_timestamp_pair_t>;


    private:
    const std::string& pcap_file;
    const std::string& timestamp_file;
    int64_t num_packets_parsed;
    inter_thread_comm_t  thread_comm;
    packet_timestamp_pair_t message;

    // thread producer related members

    public:

    void from_pcap_file();

    void push_message();

}


#endif // __PARSE_PCAP__
