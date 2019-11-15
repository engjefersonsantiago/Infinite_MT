#!/bin/bash

CACHE_L1_SIZE_L=(1024 2048 4096 8192 16384 32768)
CACHE_TYPE_L=("CacheType::LFU" "CacheType::LRU" "CacheType::RANDOM")

N_TESTS=${#CACHE_L1_SIZE_L[@]}

ORI_DIR=$PWD
for CACHE in "${CACHE_TYPE_L[@]}"; do
    for ((i=0; i < $N_TESTS; i++)); do 
	    cd ../scripts
        echo `python3.6 generate_params.py ${CACHE} ${CACHE_L1_SIZE_L[$i]} 65536`
        cd $ORI_DIR
        echo `make`
        echo `./pipeline /home/jef/Downloads/equinix-nyc.dirA.20190117-125910.UTC.anon.pcap /home/jef/Downloads/equinix-nyc.dirA.20190117-125910.UTC.anon.times >> ${CACHE_L1_SIZE_L[$i]}${CACHE}`
    done;
done
