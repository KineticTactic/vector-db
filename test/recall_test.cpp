#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

#include <vecdb/recall.hpp>

namespace {

// Calls are wrapped in an extra pair of parentheses: the braced id lists
// contain commas, which the preprocessor would otherwise read as extra macro
// arguments.

TEST(RecallAtKTest, PerfectMatchIsOne) {
    EXPECT_DOUBLE_EQ((vecdb::recall_at_k({1, 2, 3, 4}, {1, 2, 3, 4}, 4)), 1.0);
}

TEST(RecallAtKTest, IgnoresOrdering) {
    EXPECT_DOUBLE_EQ((vecdb::recall_at_k({4, 3, 2, 1}, {1, 2, 3, 4}, 4)), 1.0);
}

TEST(RecallAtKTest, DisjointResultIsZero) {
    EXPECT_DOUBLE_EQ((vecdb::recall_at_k({5, 6, 7, 8}, {1, 2, 3, 4}, 4)), 0.0);
}

TEST(RecallAtKTest, PartialOverlapIsTheHitFraction) {
    EXPECT_DOUBLE_EQ((vecdb::recall_at_k({1, 2, 9, 9}, {1, 2, 3, 4}, 4)), 0.5);
    EXPECT_DOUBLE_EQ((vecdb::recall_at_k({1, 7, 8, 9}, {1, 2, 3, 4}, 4)), 0.25);
}

TEST(RecallAtKTest, OnlyTheFirstKIdsCount) {
    // The 5th retrieved id matches, but k=4 cuts it off.
    EXPECT_DOUBLE_EQ((vecdb::recall_at_k({1, 2, 3, 9, 4}, {1, 2, 3, 4, 5}, 4)), 0.75);
}

TEST(RecallAtKTest, ZeroKThrowsInvalidArgument) {
    EXPECT_THROW((vecdb::recall_at_k({1, 2}, {1, 2}, 0)), std::invalid_argument);
}

TEST(RecallAtKTest, TooFewIdsThrowsInvalidArgument) {
    EXPECT_THROW((vecdb::recall_at_k({1, 2}, {1, 2, 3}, 3)), std::invalid_argument);
    EXPECT_THROW((vecdb::recall_at_k({1, 2, 3}, {1, 2}, 3)), std::invalid_argument);
}

}  // namespace
