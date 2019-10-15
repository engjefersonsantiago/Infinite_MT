// Pcap++
#include "parse_pcap.hpp"



/*
*   Class to read a pcap trace. 
*   Returns a pair packet,timestamp.
*
*/


void ParsePackets::from_pcap_file(){

    pcpp::PcapFileReaderDevice reader(pcap_file.c_str());

    if (!reader.open())
    {
        
        std::cout << "Error opening the pcap file\n";
        return false;
    }

    std::ifstream timestamp_reader(timestamp_file, std::ios_base::in);
    if (!timestamp_reader.is_open())
    {
        std::cout << "Error opening the timestamp file\n";
        return false;
    }


    pcpp::RawPacket rawPacket;
    while (reader.getNextPacket(rawPacket) && !timestamp_reader.eof()) {
        ++num_pkts_;

        pcpp::Packet parsedPacket(&rawPacket);        
        double timestamp;
        timestamp_reader >> timestamp;

        auto packet_timestamp_pair = std::make_pair(rawPacket,timestamp);

        

        // Send time
    }
}

