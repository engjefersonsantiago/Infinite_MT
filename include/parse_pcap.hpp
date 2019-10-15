
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


    private:
    const std::string& pcap_file;
    const std::string& timestamp_file;
    // thread producer related members
    int64_t num_packets_parsed;


    public:

    void from_pcap_file();

}


#endif // __PARSE_PCAP__
