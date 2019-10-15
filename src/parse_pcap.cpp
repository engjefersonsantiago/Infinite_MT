// Pcap++
#include "parse_pcap.hpp"



/* * 
*   Class to read a pcap trace. 
*   Returns a pair packet,timestamp.
*
*   Method Definition.
*
* */


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
        message = std::make_pair(rawPacket,timestamp);

        // Push  Packet Timestamp Pair
        push_message();

    }
}

void ParsePackets::push_message(){

    std::unique_lock<std::mutex> lock {thread_comm.mmutex};
    thread_comm.mqueue.push(message);
    thread_comm.mcond.notify_one();


}
