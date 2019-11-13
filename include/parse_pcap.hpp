
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

class ParsePackets {
    public: 


    private:
        const std::string pcap_file;
        const std::string timestamp_file;
        size_t num_packets_parsed = 0;

    // thread producer related members

    public:

        bool from_pcap_file(inter_thread_comm_t& thread_comm);

        ParsePackets(const std::string& pcap_file_, const std::string& timestamp_file_) :
           pcap_file(pcap_file_), timestamp_file(timestamp_file_) {}

};


#endif // __PARSE_PCAP__
