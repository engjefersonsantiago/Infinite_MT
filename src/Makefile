include /usr/local/etc/PcapPlusPlus.mk

# All Target
all:
	g++ $(PCAPPP_INCLUDES) -g -c -o pkt_common.o pkt_common.cpp -I ../include -std=c++17 -Wall
	g++ $(PCAPPP_INCLUDES) -g -c -o parse_pcap.o parse_pcap.cpp -I ../include -std=c++17 -Wall
	g++ $(PCAPPP_INCLUDES) -g -c -o pipeline.o pipeline.cpp -I ../include -std=c++17 -Wall
	g++ $(PCAPPP_INCLUDES) -g -c -o pipeline_seq.o pipeline_seq.cpp -I ../include -std=c++17 -Wall
	g++ $(PCAPPP_LIBS_DIR) -g -o pipeline pipeline.o parse_pcap.o pkt_common.o $(PCAPPP_LIBS) -std=c++17 -Wall
	g++ $(PCAPPP_LIBS_DIR) -g -o pipeline_seq pipeline_seq.o parse_pcap.o pkt_common.o $(PCAPPP_LIBS) -std=c++17 -Wall
	ctags -R --c++-kinds=+p --fields=+iaS --extra=+q ../include $(PCAPPP_LIBS_DIR) 
# Clean Target
clean:
	rm *.o
	rm pipeline
