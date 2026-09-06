#include "vecdb/flat_search.hpp"
#include <gtest/gtest.h>
#include <unordered_set>
#include <vector>

#include <vecdb/vector_record.hpp>
#include <vecdb/vector_store_io.hpp>

class SiftSmallTest : public testing::Test {
  protected:
    std::vector<vecdb::VectorRecord<float>> base;
    std::vector<vecdb::VectorRecord<float>> query;
    std::vector<vecdb::VectorRecord<int>> truth;

    SiftSmallTest() {
        const std::string path = std::string(DATA_DIR) + "/siftsmall/";

        base = vecdb::VectorStoreIO::read_vecs<float>(path + "base.fvecs");
        query = vecdb::VectorStoreIO::read_vecs<float>(path + "query.fvecs");
        truth = vecdb::VectorStoreIO::read_vecs<int>(path + "groundtruth.ivecs");
    }
};

// Test if the data is loaded correctly
TEST_F(SiftSmallTest, DimensionAndNumberCorrect) {
    ASSERT_EQ(base.size(), 10000);
    ASSERT_EQ(base[0].dimension(), 128u);
}

// Test Recall@100 by running flat_search
TEST_F(SiftSmallTest, FlatSearchRecall100) {
    ASSERT_EQ(query.size(), truth.size());

    constexpr int k = 100;
    double total_recall = 0.0;

    for (size_t i = 0; i < query.size(); ++i) {
        const auto nearest = vecdb::flat_search(query[i].vector, base, k);

        ASSERT_EQ(nearest.size(), k);
        ASSERT_EQ(truth[i].dimension(), k);

        std::unordered_set<int> nearest_set(nearest.begin(), nearest.end());

        int retrieved = 0;

        for (int id : truth[i].vector) {
            if (nearest_set.contains(id)) {
                ++retrieved;
            }
        }

        const double recall = static_cast<double>(retrieved) / k;

        total_recall += recall;

        EXPECT_DOUBLE_EQ(recall, 1.0) << "Query " << i << " has Recall@" << k << " = " << recall;
    }

    const double average_recall = total_recall / query.size();

    EXPECT_DOUBLE_EQ(average_recall, 1.0);
}
