import os
import sys 

def gen_params(cache_type, l1_cache_size, l2_cache_size, cache_init):
    dump_file = f"""
#include "pkt_common.hpp"
#include "policy.hpp"

static constexpr auto CACHE_INIT_STS = {cache_init};

static constexpr auto CACHE_POLICY = {cache_type};

static constexpr std::size_t L1_SIZE = {l1_cache_size};
static constexpr std::size_t L2_SIZE = {l2_cache_size};
"""
    return dump_file

def write_hpp_file (file_str):
    relative_path_hpp = "/../include/"
    current_path = os.getcwd()
    os.chdir(current_path+relative_path_hpp)
    with open("pipeline_params.hpp", 'w') as file_handler:
        file_handler.write(file_str)

if __name__ == "__main__":
    cache_type = sys.argv[1]
    l1_cache_size = sys.argv[2]
    l2_cache_size = sys.argv[3]
    cache_init = sys.argv[4]
    write_hpp_file(gen_params(cache_type, l1_cache_size, l2_cache_size, cache_init))
