#include "../include/pkt_common.hpp"
#include "../include/static_hash_map.hpp"

int main () {

    FiveTuple f1 = { "SRC_1", "DST_1", 1, 2, 3 };
    FiveTuple f2 = { "SRC_2", "DST_2", 4, 5, 6 };

    StaticHashMap<16, FiveTuple, int> hash_map;
    hash_map[f1] = 14;

    auto a = hash_map.find(f1);
    std::cout << ((a != std::end(hash_map)) ? "Found\n" : "Not Found\n");

    for (const auto& [key, value]: hash_map)
        std::cout << "Key: " << key << " Value: " << value << '\n';

}

