// STL
#include<chrono>
#include<iostream>
#include<fstream>

// Pcap++
#include "parse_pcap.hpp"

using namespace std::literals::chrono_literals;

/* *
*   Class to read a pcap trace.
*   Returns a pair packet,timestamp.
*
*   Method Definition.
*
* */


bool ParsePackets::from_pcap_file(inter_thread_comm_t& thread_comm, const nano_second_t sleep_time){

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
        ++num_packets_parsed;

        pcpp::Packet parsedPacket(&rawPacket);
        double timestamp;
        timestamp_reader >> timestamp;

        // Push  Packet Timestamp Pair
        thread_comm.push_message(std::make_pair(parsedPacket, timestamp));

        debug(
        std::cout << "Thread ID " << std::this_thread::get_id() << " processed " << num_packets_parsed << " packets\n";
        )

        std::this_thread::sleep_for(sleep_time);

    }
    thread_comm.set_done();
    return true;
}


