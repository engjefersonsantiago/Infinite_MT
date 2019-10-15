include /usr/local/etc/PcapPlusPlus.mk

# All Target
all:
	g++ $(PCAPPP_INCLUDES) -c -o pipeline.o pipeline.cpp -I ../include -std=c++17 -Wall
	g++ $(PCAPPP_LIBS_DIR) -o pipeline pipeline.o $(PCAPPP_LIBS) -std=c++17 -Wall
	ctags -R --c++-kinds=+p --fields=+iaS --extra=+q ../include $(PCAPPP_LIBS_DIR) 
# Clean Target
clean:
	rm *.o
	rm pipeline
