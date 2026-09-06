#include "vecdb/flat_search.hpp"
#include <gtest/gtest.h>
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

// Test Recal@100 by running flat_search
TEST_F(SiftSmallTest, FlatSearchRecall100) {
    ASSERT_EQ(query.size(), truth.size());

    // Our dataset contains 100 nearest vectors for each query
    int k = 100;

    for (size_t i = 0; i < query.size(); i++) {
        std::vector<int> nearest = vecdb::flat_search(query[i].vector, base, k);

        EXPECT_EQ(nearest.size(), truth[i].dimension());
        EXPECT_EQ(nearest, truth[i].vector);
    }
}
