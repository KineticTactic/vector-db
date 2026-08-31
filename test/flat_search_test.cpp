#include <gtest/gtest.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <vecdb/distance.hpp>
#include <vecdb/flat_search.hpp>
#include <vecdb/vector_record.hpp>

namespace {

/// Builds records with ids 0..n-1 in the order the vectors are given.
std::vector<vecdb::VectorRecord<float>> make_base(
    const std::vector<std::vector<float>> &vectors) {
    std::vector<vecdb::VectorRecord<float>> base;
    base.reserve(vectors.size());
    for (std::size_t i = 0; i < vectors.size(); ++i) {
        base.push_back({static_cast<int>(i), vectors[i], {}});
    }
    return base;
}

/// Points spaced along the x axis. Distances from the origin are 0, 1, 4, 9,
/// 100 - worked out by hand rather than taken from the implementation.
const std::vector<std::vector<float>> kLine = {
    {0.0f, 0.0f}, {1.0f, 0.0f}, {2.0f, 0.0f}, {3.0f, 0.0f}, {10.0f, 0.0f}};

TEST(FlatSearchTest, ReturnsNearestNeighboursInAscendingDistanceOrder) {
    const auto base = make_base(kLine);
    const std::vector<float> query = {0.0f, 0.0f};

    EXPECT_EQ(vecdb::flat_search(query, base, 3), (std::vector<int>{0, 1, 2}));
}

TEST(FlatSearchTest, FindsNeighboursAroundAnInteriorQuery) {
    const auto base = make_base(kLine);
    // Query at x = 2.9: distances are 8.41, 3.61, 0.81, 0.01, 50.41.
    const std::vector<float> query = {2.9f, 0.0f};

    EXPECT_EQ(vecdb::flat_search(query, base, 4), (std::vector<int>{3, 2, 1, 0}));
}

TEST(FlatSearchTest, KEqualToDatasetSizeReturnsEverything) {
    const auto base = make_base(kLine);
    const std::vector<float> query = {0.0f, 0.0f};

    const auto found = vecdb::flat_search(query, base, base.size());

    ASSERT_EQ(found.size(), base.size());
    EXPECT_EQ(found, (std::vector<int>{0, 1, 2, 3, 4}));
}

TEST(FlatSearchTest, ResultIsSortedByDistance) {
    const auto base = make_base({{5.0f, 5.0f},
                                 {-2.0f, 7.0f},
                                 {0.0f, 1.0f},
                                 {9.0f, -3.0f},
                                 {1.0f, 1.0f},
                                 {-8.0f, 0.5f}});
    const std::vector<float> query = {0.5f, 0.5f};

    const auto found = vecdb::flat_search(query, base, 4);

    ASSERT_EQ(found.size(), 4u);
    for (std::size_t i = 1; i < found.size(); ++i) {
        const float previous = vecdb::squared_l2(query, base[found[i - 1]].vector);
        const float current = vecdb::squared_l2(query, base[found[i]].vector);
        EXPECT_LE(previous, current) << "result not ordered at position " << i;
    }
}

TEST(FlatSearchTest, TiedDistancesReturnDistinctIdsFromTheTiedSet) {
    // Four points equidistant from the origin, plus one far away.
    const auto base = make_base({{1.0f, 0.0f},
                                 {-1.0f, 0.0f},
                                 {0.0f, 1.0f},
                                 {0.0f, -1.0f},
                                 {50.0f, 50.0f}});
    const std::vector<float> query = {0.0f, 0.0f};

    auto found = vecdb::flat_search(query, base, 3);

    ASSERT_EQ(found.size(), 3u);
    std::sort(found.begin(), found.end());
    EXPECT_EQ(std::adjacent_find(found.begin(), found.end()), found.end()) << "duplicate ids";
    for (const int id : found) {
        EXPECT_NE(id, 4) << "the distant point must never beat a tied neighbour";
    }
}

TEST(FlatSearchTest, ZeroKThrowsInvalidArgument) {
    const auto base = make_base(kLine);
    EXPECT_THROW((vecdb::flat_search({0.0f, 0.0f}, base, 0)), std::invalid_argument);
}

TEST(FlatSearchTest, KLargerThanDatasetThrowsInvalidArgument) {
    const auto base = make_base(kLine);
    EXPECT_THROW((vecdb::flat_search({0.0f, 0.0f}, base, base.size() + 1)), std::invalid_argument);
}

TEST(FlatSearchTest, DimensionMismatchThrowsInvalidArgument) {
    const auto base = make_base(kLine);
    const std::vector<float> query = {1.0f, 2.0f, 3.0f};

    EXPECT_THROW(vecdb::flat_search(query, base, 2), std::invalid_argument);
}

TEST(SquaredL2Test, MatchesHandComputedDistance) {
    // (1,2,3) vs (4,6,3): 9 + 16 + 0 = 25.
    EXPECT_FLOAT_EQ((vecdb::squared_l2({1.0f, 2.0f, 3.0f}, {4.0f, 6.0f, 3.0f})), 25.0f);
    EXPECT_FLOAT_EQ((vecdb::squared_l2({1.0f, 2.0f}, {1.0f, 2.0f})), 0.0f);
}

TEST(SquaredL2Test, DimensionMismatchThrowsInvalidArgument) {
    EXPECT_THROW((vecdb::squared_l2({1.0f}, {1.0f, 2.0f})), std::invalid_argument);
}

}  // namespace
