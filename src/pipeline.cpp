#include<thread>

#include "main_pipe.hpp"


using passed_pkt_t = std::pair<pcpp::Packet, double>;
using in2main_comm_t = ThreadCommunication<passed_pkt_t>;

int main() {

    in2main_comm_t in2main_comm; 
    MainPipe<1024, int, in2main_comm_t> main_pipe;

    std::thread main_pipe_thread(&MainPipe<1024, int, in2main_comm_t>::process_packet, main_pipe, std::ref(in2main_comm)); 

    main_pipe_thread.join();

}
