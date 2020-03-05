// STL
#include<chrono>
#include<iostream>

#include "parse_pcap.hpp"

//{ Local helper classes and functions for parsing.
class ReaderWithProgressMeter : public BufferedReader<> {
    public:
        ReaderWithProgressMeter(const std::string& filename) :
                BufferedReader(filename, std::ios::binary) {
        }

        void showProgress(std::ostream& os = std::cout) {
            //NOTE: If using text istream, do not use tellg at each packet, it is slow.
            if ((++n_processed_) % check_process_every_n == 0) {
                unsigned progress = unsigned(tellg()*100 / file_size_);
                if (progress != last_progress_) {
                    last_progress_ = progress;
                    if (progress % 10 == 0)
                        os << progress << "%";
                    else
                        os << ".";
                    os << std::flush;
                }
            }
        }

        bool isSomethingRemaining() {
            return !ws(*this).isEnd();
        }

    private:
        static constexpr unsigned check_process_every_n = 10000;

        std::size_t n_processed_ = 0;
        std::streamoff file_size_ = getFileSize();
        unsigned last_progress_ = 0;
};

class RelativeTimestampParser : public ReaderWithProgressMeter {
        static constexpr int64_t no_offset_yet = std::numeric_limits<int64_t>::max();

    public:
        RelativeTimestampParser(const std::string& filename) :
            ReaderWithProgressMeter(filename) {}
        double getNextTimestamp() {
            static constexpr std::size_t longest_integer_chars = 20;
	        ws(*this);
            if (timestamp_offset_ == no_offset_yet) {
                hasCharsLeft(longest_integer_chars);
                const char* text_begin = getPointer();
                timestamp_offset_ = peekInteger(text_begin, text_begin+longest_integer_chars);
            }
	        return readDouble(*this, timestamp_offset_);
        }

    private:
        int64_t timestamp_offset_ = no_offset_yet;
};

class PcapFileReader : private BufferedReader<> {
    public:
        // Structs almost directly from https://wiki.wireshark.org/Development/LibpcapFileFormat
        struct pcap_hdr_s {
	        uint32_t magic_number;   // magic number
	        uint16_t version_major;  // major version number
	        uint16_t version_minor;  // minor version number
	        int32_t  thiszone;       // GMT to local correction
	        uint32_t sigfigs;        // accuracy of timestamps
	        uint32_t snaplen;        // max length of captured packets, in octets
	        uint32_t network;        // data link type
	
	        static constexpr uint32_t magic_for_microsecond = 0xa1b2c3d4;
	        static constexpr uint32_t magic_for_nanosecond  = 0xa1b23c4d;
	        bool is_native_microsecond_format() const { return magic_number == magic_for_microsecond; }
	        bool is_native_nanosecond_format()  const { return magic_number == magic_for_nanosecond; }
	        bool is_native_endian() const { return is_native_microsecond_format() || is_native_nanosecond_format(); }
        };
        struct pcaprec_hdr_s {
	        uint32_t ts_sec;         // timestamp seconds
	        uint32_t ts_usec;        // timestamp microseconds (or nanoseconds)
	        uint32_t incl_len;       // number of octets of packet saved in file
	        uint32_t orig_len;       // actual length of packet
        };

        PcapFileReader(const std::string& filename) :
            BufferedReader(filename, std::ios::binary)
        {
            if (!getStream().is_open())
                throw std::ifstream::failure("Error opening the pcap file");
            file_header_ = readAs<pcap_hdr_s>();
        }

        bool getNextPacket(pcpp::RawPacket& rawPacket_out) {
            *this += previous_packet_length_;  // Head is moved when reading next packet, keeping current packet in the buffer, so we can return it without copy.
            auto* packet = peekAs<pcaprec_hdr_s*>();
            if (!packet || !hasCharsLeft(previous_packet_length_ = sizeof(pcaprec_hdr_s) + packet->incl_len)) {  // Possibly invalidates the pointer.
                previous_packet_length_ = 0;
                return false;
            }
            packet = peekAs<pcaprec_hdr_s*>();  // We thus get the pointer again.

            rawPacket_out.setRawData(reinterpret_cast<const uint8_t*>(packet+1), packet->incl_len, get_packet_timestamp(*packet), pcpp::LINKTYPE_DLT_RAW1, packet->orig_len);
            return true;
        }

        using BufferedReader::tellg;
        using BufferedReader::getFileSize;

    private:
        timespec get_packet_timestamp(const pcaprec_hdr_s& packet_header) {
            return { packet_header.ts_sec, long(packet_header.ts_usec) * (is_nanosecond_format_ ? 1 : 1000) };
        }

        pcap_hdr_s file_header_;
        bool is_nanosecond_format_;
        size_t previous_packet_length_ = 0;
};

static std::ofstream openBinaryOutputFile(const std::string& filename)
{
    std::ofstream output_file(filename, std::ios::binary);
    if (output_file.fail())
        throw std::ofstream::failure("Error opening output file '" + filename + "'");
    output_file.exceptions(std::ios::failbit);
    return output_file;
}
//}

void preparsePackets(const std::string& pcap_filename, const std::string& timestamp_filename, const std::string& output_filename)
{
    PcapFileReader pcap_reader(pcap_filename);
    RelativeTimestampParser timestamp_reader(timestamp_filename);
    std::ofstream preparsed_packet_writer = openBinaryOutputFile(output_filename);

    std::cout << "Preparsing: " << std::flush;
    ParsedPacket parsedPacket{}; // Ensure zero initialization.
    for (pcpp::RawPacket rawPacket(nullptr, 0, timespec{}, false); pcap_reader.getNextPacket(rawPacket) && timestamp_reader.isSomethingRemaining(); )
    {
        pcpp::Packet packet(&rawPacket, pcpp::OsiModelTransportLayer);
        parsedPacket.timestamp = timestamp_reader.getNextTimestamp();
        std::tie(parsedPacket.five_tuple, parsedPacket.pkt_size) = create_five_tuple_from_packet(packet);
        parsedPacket.write_to(preparsed_packet_writer);

        timestamp_reader.showProgress();
    }
    std::cout << " done." << std::endl;
}

/* *
*   Class to read a pcap trace.
*   Returns a pair packet,timestamp.
*
*   Method Definition.
*
* */


bool ParsePackets::from_pcap_file(const bool run_forever, inter_thread_comm_t& thread_comm)
{
    if (!reader.getStream().is_open())
    {
        std::cout << "Error opening the pcap file\n";
        return false;
    }

    debug(std::cout << "===========================================\n";)
    for (const ParsedPacket* parsedPacket; (parsedPacket = reader.peekAs<ParsedPacket*>()) != nullptr; reader += sizeof(ParsedPacket))
    {
        ++num_packets_parsed;
        
        debug(std::cout << "Thread ID " << std::this_thread::get_id() << " processed " << num_packets_parsed << " packets\n";)

        // Push  Packet Timestamp Pair
        if (!run_forever)
        {
            thread_comm.push_message(*parsedPacket);
            reader += sizeof(ParsedPacket);
            return true;
        } else 
        {
            thread_comm.push_message_two_notify(*parsedPacket);
        }

    }
    thread_comm.set_done();
    return false;
}
