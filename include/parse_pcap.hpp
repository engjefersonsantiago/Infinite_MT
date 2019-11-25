#include<fstream>

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
        size_t num_packets_parsed = 0;
        pcpp::PcapFileReaderDevice reader;
        std::ifstream timestamp_reader;  
        // thread producer related members

    public:

        bool from_pcap_file(const bool run_forever, inter_thread_comm_t& thread_comm);

        ParsePackets(const std::string& pcap_file_, const std::string& timestamp_file_) :
                        reader(pcap_file_.c_str()),
                        timestamp_reader(timestamp_file_, std::ios_base::in) {}

};


#endif // __PARSE_PCAP__
