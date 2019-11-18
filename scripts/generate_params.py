import os
import sys

def gen_params(l1_slowdown_factor, l2_slowdown_factor, cpu_slowdown_factor, l1_cache_init, l2_cache_init, l1_cache_type, l2_cache_type, l1_cache_size, l2_cache_size, l1_stats_size, l2_stats_size):
    dump_file = f"""
#ifndef PIPELINE_PARAMS_HPP
#define PIPELINE_PARAMS_HPP

// Types
enum class CacheType
{{
    OPT,
    LRU,
    LFU,
    LRFU,
    RANDOM
}};

enum class CacheInit
{{
    EMPTY,
    FULL
}};

// Constants
static constexpr std::size_t CACHE_L1_PROC_SLOWDOWN_FACTOR = {l1_slowdown_factor};
static constexpr std::size_t CACHE_L2_PROC_SLOWDOWN_FACTOR = {l2_slowdown_factor};
static constexpr std::size_t CACHE_HOST_PROC_SLOWDOWN_FACTOR = {cpu_slowdown_factor};

static constexpr auto L1_CACHE_INIT_STS = {l1_cache_init};
static constexpr auto L2_CACHE_INIT_STS = {l2_cache_init};

static constexpr auto L1_CACHE_POLICY = {l1_cache_type};
static constexpr auto L2_CACHE_POLICY = {l2_cache_type};

static constexpr std::size_t L1_CACHE_SIZE = {l1_cache_size};
static constexpr std::size_t L2_CACHE_SIZE = {l2_cache_size};

static constexpr std::size_t L1_CACHE_STATS_SIZE = {l1_stats_size};
static constexpr std::size_t L2_CACHE_STATS_SIZE = {l2_stats_size};

#endif
    """
    return dump_file

def write_hpp_file (file_str):
    relative_path_hpp = "/../include/"
    current_path = os.getcwd()
    os.chdir(current_path+relative_path_hpp)
    with open("pipeline_params.hpp", 'w') as file_handler:
        file_handler.write(file_str)

if __name__ == "__main__":
    l1_slowdown_factor = sys.argv[1]
    l2_slowdown_factor = sys.argv[2]
    cpu_slowdown_factor = sys.argv[3]
    l1_cache_init = sys.argv[4]
    l2_cache_init = sys.argv[5]
    l1_cache_type = sys.argv[6]
    l2_cache_type = sys.argv[7]
    l1_cache_size = sys.argv[8]
    l2_cache_size = sys.argv[9]
    l1_stats_size = sys.argv[10]
    l2_stats_size = sys.argv[11]
    write_hpp_file(gen_params(l1_slowdown_factor, l2_slowdown_factor, cpu_slowdown_factor, l1_cache_init, l2_cache_init, l1_cache_type, l2_cache_type, l1_cache_size, l2_cache_size, l1_stats_size, l2_stats_size))
