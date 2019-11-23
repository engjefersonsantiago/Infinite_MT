// STL
#include<chrono>
#include<iostream>

#include "parse_pcap.hpp"
#include "pkt_common.hpp"

/* *
*   Class to read a pcap trace.
*   Returns a pair packet,timestamp.
*
*   Method Definition.
*
* */


bool ParsePackets::from_pcap_file(const bool run_forever, inter_thread_comm_t& thread_comm)
{
    if (!reader.open())
    {
        std::cout << "Error opening the pcap file\n";
        return false;
    }

    if (!timestamp_reader.is_open())
    {
        std::cout << "Error opening the timestamp file\n";
        return false;
    }

    pcpp::RawPacket rawPacket;
    while (reader.getNextPacket(rawPacket) && !timestamp_reader.eof())
    {
        ++num_packets_parsed;

        pcpp::Packet parsedPacket(&rawPacket);
        double timestamp;
        timestamp_reader >> timestamp;
        
        debug(std::cout << "Thread ID " << std::this_thread::get_id() << " processed " << num_packets_parsed << " packets\n";)

        // Push  Packet Timestamp Pair
        if (!run_forever)
        {
            thread_comm.push_message(std::make_pair(parsedPacket, timestamp));
            return true;
        } else 
        {
            thread_comm.push_message_two_notify(std::make_pair(parsedPacket, timestamp));
        }

    }
    thread_comm.set_done();
    return false;
}
