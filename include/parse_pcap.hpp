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
#include "buffered_reader.hpp"


void preparsePackets(const std::string& pcap_filename, const std::string& timestamp_filename, const std::string& output_filename);


class ParsePackets {
    public: 


    private:
        size_t num_packets_parsed = 0;
        BufferedReader<> reader;
        // thread producer related members

    public:

        bool from_pcap_file(const bool run_forever, inter_thread_comm_t& thread_comm);

        ParsePackets(const std::string& pcap_pre_file_) :
                        reader(pcap_pre_file_, std::ios::binary) {}

};


#endif // __PARSE_PCAP__
