#include <memory>
#include <thread>
#include <gtest/gtest.h>

#include "pkt_common.hpp"
#include "lookup_table.hpp"

using lookup_table_t = LookupTable<8, int>;

struct LookupTableTest : testing::Test {
     std::unique_ptr<lookup_table_t> lookup_table = std::make_unique<lookup_table_t>();
};

// Test key insertion
TEST_F(LookupTableTest, TestInsertion) {
	FiveTuple ft { "IP1", "IP2", 4, 5, 6 };
    int val = 1;
	EXPECT_TRUE(lookup_table->insert(ft, val));
}

// Test key deletion
TEST_F(LookupTableTest, TestDeletion) {
	FiveTuple ft { "IP1", "IP2", 4, 5, 6 };
    int val = 1;
	lookup_table->insert(ft, val);
	EXPECT_TRUE(lookup_table->remove(ft));
}

// Test key replacement
TEST_F(LookupTableTest, TestReplace) {
	FiveTuple ft { "IP1", "IP2", 4, 5, 6 };
	FiveTuple ft2 { "IP2", "IP1", 4, 5, 6 };
    int val = 1;
	lookup_table->insert(ft, val);
	EXPECT_TRUE(lookup_table->replace(ft,ft2,val));
}

// Test key replacement
TEST_F(LookupTableTest, TestConcurrence) {
	FiveTuple ft { "IP1", "IP2", 4, 5, 6 };
	FiveTuple ft2 { "IP2", "IP1", 4, 5, 6 };
    int val = 1;

    auto add_key = [&](const auto& key, const auto& val) { lookup_table->insert(key, val); };
    auto replace_key = [&](const auto& old_key, const auto& new_key, const auto& new_val) {
        lookup_table->replace(old_key, new_key, new_val);
    };

    std::thread t1 (add_key, std::ref(ft), std::ref(val));
    std::thread t2 (replace_key, std::ref(ft), std::ref(ft2), std::ref(val));

    t1.join();
    t2.join();

	EXPECT_EQ(lookup_table->occupancy(), 1);
}


int main (int argc, char* argv[]) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
