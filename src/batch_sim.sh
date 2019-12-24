#!/bin/bash
L1_SLOWDOWN_FACTOR=(1)
L2_SLOWDOWN_FACTOR=(10)
CPU_SLOWDOWN_FACTOR=(1 10 100)

L1_CACHE_INIT=("CacheInit::EMPTY")
L2_CACHE_INIT=("CacheInit::EMPTY")

L1_CACHE_TYPE_L=("CacheType::LFU" "CacheType::LFU_MODIF") # "CacheType::LRU" "CacheType::RANDOM")
L2_CACHE_TYPE_L=("CacheType::LFU") # "CacheType::RANDOM")

LFU_COUNTER_TYPE=("CounterType::PKTS" "CounterType::BYTES")

CACHE_L1_SIZE_L=(64 128 256 512 1024 2048 4096 8192) #16384 32768)
CACHE_L2_SIZE_L=(1 1 1 1 1 1 1 1) #16384 32768)

CACHE_L1_STATS_SIZE_L=(64 128 256 512 1024 2048 4096 8192)
CACHE_L2_STATS_SIZE_L=(1 1 1 1 1 1 1 1 1)

N_TESTS=${#CACHE_L1_SIZE_L[@]}

ORI_DIR=$PWD
for COUNTER in "${LFU_COUNTER_TYPE[@]}"; do
    for CPU_SLOWDOWN in "${CPU_SLOWDOWN_FACTOR[@]}"; do
        for ((i=0; i < $N_TESTS; i++)); do
            for L2_CACHE in "${L2_CACHE_TYPE_L[@]}"; do
                for L1_CACHE in "${L1_CACHE_TYPE_L[@]}"; do
    	            cd ../scripts
                    echo `python3.6 generate_params.py ${L1_SLOWDOWN_FACTOR} ${L2_SLOWDOWN_FACTOR} ${CPU_SLOWDOWN}   \
                                                        ${L1_CACHE_INIT} ${L2_CACHE_INIT}                                   \
                                                        ${L1_CACHE} ${L2_CACHE}                       \
                                                        ${CACHE_L1_SIZE_L[$i]} ${CACHE_L2_SIZE_L[$i]}                       \
                                                        ${CACHE_L1_STATS_SIZE_L[$i]} ${CACHE_L1_STATS_SIZE_L[$i]} ${COUNTER}\
                                                        `
                    cd $ORI_DIR
                    echo `make`
                    echo `./pipeline_seq $1 $2 >> ${L1_SLOWDOWN_FACTOR}-${L2_SLOWDOWN_FACTOR}-${CPU_SLOWDOWN}-${L1_CACHE_INIT}-${L2_CACHE_INIT}-${L1_CACHE}-${L2_CACHE}-${CACHE_L1_SIZE_L[$i]}-${CACHE_L2_SIZE_L[$i]}-${CACHE_L1_STATS_SIZE_L[$i]}-${CACHE_L2_STATS_SIZE_L[$i]}-${COUNTER}`
                done
            done
        done
    done
done
