include /usr/local/etc/PcapPlusPlus.mk

test:
	cd ../src/ && make
	g++ -std=c++17 test_lookup_table.cpp -I ../include $(PCAPPP_INCLUDES) $(PCAPPP_LIBS_DIR) -lgtest -lpthread -o test_lookup_table.out
	cd ../	
	python3.6 ../test/pcap_generation/generate_pcap.py	
	g++ -std=c++17 test_parse_pcap.cpp ../src/parse_pcap.o ../src/pkt_common.o -I ../include $(PCAPPP_INCLUDES) $(PCAPPP_LIBS) -lgtest -lpthread -o test_parse_pcap.out
clean:
	rm *.out
