
#ifndef PIPELINE_PARAMS_HPP
#define PIPELINE_PARAMS_HPP

// Types
enum class CacheType
{
    OPT,
    LRU,
    LFU,
    LRFU,
    RANDOM
};

enum class CacheInit
{
    EMPTY,
    FULL
};

// Constants
static constexpr std::size_t CACHE_L1_PROC_SLOWDOWN_FACTOR = 1;
static constexpr std::size_t CACHE_L2_PROC_SLOWDOWN_FACTOR = 10;
static constexpr std::size_t CACHE_HOST_PROC_SLOWDOWN_FACTOR = 10;

static constexpr auto L1_CACHE_INIT_STS = CacheInit::FULL;
static constexpr auto L2_CACHE_INIT_STS = CacheInit::EMPTY;

static constexpr auto L1_CACHE_POLICY = CacheType::RANDOM;
static constexpr auto L2_CACHE_POLICY = CacheType::LFU;

static constexpr std::size_t L1_CACHE_SIZE = 1024;
static constexpr std::size_t L2_CACHE_SIZE = 128;

static constexpr std::size_t L1_CACHE_STATS_SIZE = 32;
static constexpr std::size_t L2_CACHE_STATS_SIZE = 128;

#endif
    
