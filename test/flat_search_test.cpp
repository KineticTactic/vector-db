#include <gtest/gtest.h>
#include <vecdb/flat_search.hpp>
#include <vector>

TEST(FlatSearchTest, ReturnsNearestIdsInOrder) {
    std::vector<float> query{0, 0};
    std::vector<vecdb::VectorRecord<float>> base{
        {0, {10, 10}, {}},
        {1, {1, 1}, {}},
        {2, {3, 3}, {}},
        {3, {0.5f, 0.5f}, {}},
    };
    std::vector<int> result = vecdb::flat_search(query, base, 3);
    EXPECT_EQ(result, (std::vector<int>{3, 1, 2}));
}

TEST(FlatSearchTest, KOneReturnsClosestVector) {
    std::vector<float> query{0};
    std::vector<vecdb::VectorRecord<float>> base{
        {0, {5}, {}},
        {1, {2}, {}},
        {2, {8}, {}},
    };
    std::vector<int> result = vecdb::flat_search(query, base, 1);
    EXPECT_EQ(result, (std::vector<int>{1}));
}

TEST(FlatSearchTest, EqualDistancesRetainEarlierCandidate) {    // no actual tie-breaker, returns earlier vector in case of tie
    std::vector<float> query{0};
    std::vector<vecdb::VectorRecord<float>> base{
        {0, {-1}, {}},
        {1, {1}, {}},
    };
    std::vector<int> result = vecdb::flat_search(query, base, 1);
    EXPECT_EQ(result, (std::vector<int>{0}));
}

TEST(FlatSearchTest, ZeroKInvalid) {
    std::vector<float> query{0};
    std::vector<vecdb::VectorRecord<float>> base{
        {0, {1}, {}},
    };
    EXPECT_THROW(
        vecdb::flat_search(query, base, 0),
        std::invalid_argument
    );
}

TEST(FlatSearchTest, KGreaterThanBaseSizeInvalid) {
    std::vector<float> query{0};
    std::vector<vecdb::VectorRecord<float>> base{
        {0, {1}, {}},
    };
    EXPECT_THROW(
        vecdb::flat_search(query, base, 2),
        std::invalid_argument
    );
}

TEST(FlatSearchTest, DimensionMismatchThrows) {
    std::vector<float> query{0, 1};
    std::vector<vecdb::VectorRecord<float>> base{
        {0, {1}, {}},
    };
    EXPECT_THROW(
        vecdb::flat_search(query, base, 1),
        std::invalid_argument
    );
}