
#include "pkt_common.hpp"
#include "policy.hpp"

static constexpr auto CACHE_POLICY = CacheType::RANDOM;

static constexpr std::size_t L1_SIZE = 1024;
static constexpr std::size_t L2_SIZE = 65536;
    