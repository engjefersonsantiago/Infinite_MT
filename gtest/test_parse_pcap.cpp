#include <memory>
#include <thread>
#include <mutex>
#include <gtest/gtest.h>

#include "pkt_common.hpp"
#include "parse_pcap.hpp"


static const std::string PCAP_FILE = "test_12_pkts_5_tuples.pcap";
static const std::string TIMES_FILE = "test_12_pkts_5_tuples_.times";

struct ParsePacketsTest : testing::Test {
     std::unique_ptr<ParsePackets> parse_pcap = std::make_unique<ParsePackets>(PCAP_FILE, TIMES_FILE);
};

// Test key insertion
TEST_F(ParsePacketsTest, TestParsing) {
    inter_thread_comm_t	thread_comm;
    int pkt_cnt = 0;

    const auto sleep = nano_second_t(100000);
    auto wait_pkt = [&]() { 
        while (!thread_comm.get_done()) {
            std::unique_lock lck {thread_comm.mmutex};
            auto r = thread_comm.mcond.wait_for(lck, nano_second_t(100*sleep));
            if (r == std::cv_status::no_timeout) {
                pkt_cnt++;
            }
        }
    };

    std::thread tp(&ParsePackets::from_pcap_file, *parse_pcap, std::ref(thread_comm),std::ref(sleep));
    std::thread tc(wait_pkt);

    tp.join();    
    tc.join();    

    EXPECT_EQ(pkt_cnt, 12*5);
}

int main (int argc, char* argv[]) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
