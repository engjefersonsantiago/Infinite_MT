
#include "pkt_common.hpp"
#include "policy.hpp"

static constexpr auto CACHE_POLICY = CacheType::LFU;

static constexpr std::size_t L1_SIZE = 2048;
static constexpr std::size_t L2_SIZE = 65536;
    