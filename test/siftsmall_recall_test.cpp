#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <vecdb/flat_search.hpp>
#include <vecdb/recall.hpp>
#include <vecdb/vector_record.hpp>
#include <vecdb/vector_store_io.hpp>

#ifndef VECDB_DATA_DIR
#define VECDB_DATA_DIR "data/siftsmall"
#endif

namespace {

constexpr std::size_t kNeighbours = 100;
constexpr std::size_t kExpectedBase = 10000;
constexpr std::size_t kExpectedQueries = 100;
constexpr std::size_t kExpectedDimension = 128;

/// The Phase 0 correctness baseline: an exact scan of the whole dataset must
/// reproduce the published ground truth exactly, so Recall@100 == 1.0.
/// Skipped when the dataset has not been fetched, so the rest of the suite
/// still runs on a clean checkout.
TEST(SiftSmallTest, RecallAt100IsOne) {
    const std::filesystem::path dir(VECDB_DATA_DIR);
    const auto base_path = dir / "siftsmall_base.fvecs";
    const auto query_path = dir / "siftsmall_query.fvecs";
    const auto groundtruth_path = dir / "siftsmall_groundtruth.ivecs";

    if (!std::filesystem::exists(base_path) || !std::filesystem::exists(query_path) ||
        !std::filesystem::exists(groundtruth_path)) {
        GTEST_SKIP() << "siftsmall not found in " << dir.string()
                     << "; fetch it with scripts/fetch_siftsmall.ps1 (or .sh)";
    }

    const auto base = vecdb::VectorStoreIO::read_vecs<float>(base_path.string());
    const auto queries = vecdb::VectorStoreIO::read_vecs<float>(query_path.string());
    const auto groundtruth = vecdb::VectorStoreIO::read_vecs<std::int32_t>(
        groundtruth_path.string());

    ASSERT_EQ(base.size(), kExpectedBase);
    ASSERT_EQ(queries.size(), kExpectedQueries);
    ASSERT_EQ(groundtruth.size(), kExpectedQueries);
    ASSERT_EQ(base[0].dimension(), kExpectedDimension);
    ASSERT_EQ(queries[0].dimension(), kExpectedDimension);
    ASSERT_GE(groundtruth[0].dimension(), kNeighbours);

    double recall_sum = 0.0;
    for (std::size_t i = 0; i < queries.size(); ++i) {
        const std::vector<int> found = vecdb::flat_search(queries[i].vector, base, kNeighbours);

        const auto &row = groundtruth[i].vector;
        const std::vector<int> expected(row.begin(), row.end());

        const double recall = vecdb::recall_at_k(found, expected, kNeighbours);
        EXPECT_DOUBLE_EQ(recall, 1.0) << "query " << i << " missed ground-truth neighbours";
        recall_sum += recall;
    }

    EXPECT_DOUBLE_EQ(recall_sum / static_cast<double>(queries.size()), 1.0);
}

}  // namespace
