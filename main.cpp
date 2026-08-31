#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <print>
#include <string>
#include <vector>

#include <vecdb/flat_search.hpp>
#include <vecdb/recall.hpp>
#include <vecdb/vector_record.hpp>
#include <vecdb/vector_store_io.hpp>

// Absolute path to the in-tree dataset, baked in by CMake at configure time.
#ifndef VECDB_DATA_DIR
#define VECDB_DATA_DIR "data/siftsmall"
#endif

namespace {

constexpr std::size_t kNeighbours = 100;
constexpr const char *kBaseFile = "siftsmall_base.fvecs";
constexpr const char *kQueryFile = "siftsmall_query.fvecs";
constexpr const char *kGroundTruthFile = "siftsmall_groundtruth.ivecs";

/// Picks the directory to load the dataset from.
///
/// An explicit command-line argument always wins. Otherwise the plain
/// working-directory-relative path is tried first (so `./build/vector-db-main`
/// from the repo root behaves as documented), falling back to the source-tree
/// path baked in at configure time - multi-config generators put the binary in
/// build/<config>/, where a relative "data/siftsmall" resolves to nothing.
std::filesystem::path resolve_data_dir(const char *requested) {
    if (requested != nullptr) {
        return std::filesystem::path(requested);
    }

    const std::filesystem::path relative = std::filesystem::path("data") / "siftsmall";
    if (std::filesystem::exists(relative / kBaseFile)) {
        return relative;
    }

    return std::filesystem::path(VECDB_DATA_DIR);
}

}  // namespace

/// Phase 0 validation run: load siftsmall, brute-force search every query, and
/// report Recall@100 against the provided ground truth. Exits non-zero if the
/// exact search fails to reproduce the ground truth, so it doubles as a gate.
int main(int argc, char **argv) {
    const std::filesystem::path data_dir = resolve_data_dir(argc > 1 ? argv[1] : nullptr);

    try {
        std::println("Loading siftsmall from '{}' ...", data_dir.string());

        const auto base =
            vecdb::VectorStoreIO::read_vecs<float>((data_dir / kBaseFile).string());
        const auto queries =
            vecdb::VectorStoreIO::read_vecs<float>((data_dir / kQueryFile).string());
        const auto groundtruth =
            vecdb::VectorStoreIO::read_vecs<std::int32_t>((data_dir / kGroundTruthFile).string());

        std::println("  base:        {} x {}", base.size(), base.empty() ? 0 : base[0].dimension());
        std::println("  query:       {} x {}", queries.size(),
                     queries.empty() ? 0 : queries[0].dimension());
        std::println("  groundtruth: {} x {}", groundtruth.size(),
                     groundtruth.empty() ? 0 : groundtruth[0].dimension());

        if (queries.size() != groundtruth.size()) {
            std::println("error: {} queries but {} ground-truth rows", queries.size(),
                         groundtruth.size());
            return 1;
        }

        std::println("\nRunning flat_search with k={} over {} queries ...", kNeighbours,
                     queries.size());

        double recall_sum = 0.0;
        double recall_min = 1.0;
        const auto start = std::chrono::steady_clock::now();

        for (std::size_t i = 0; i < queries.size(); ++i) {
            const std::vector<int> found =
                vecdb::flat_search(queries[i].vector, base, kNeighbours);

            const auto &row = groundtruth[i].vector;
            const std::vector<int> expected(row.begin(), row.end());

            const double recall = vecdb::recall_at_k(found, expected, kNeighbours);
            recall_sum += recall;
            recall_min = (recall < recall_min) ? recall : recall_min;
        }

        const auto elapsed = std::chrono::steady_clock::now() - start;
        const double total_ms =
            std::chrono::duration<double, std::milli>(elapsed).count();
        const double mean_recall = recall_sum / static_cast<double>(queries.size());

        std::println("\n  mean Recall@{}: {:.6f}", kNeighbours, mean_recall);
        std::println("  min  Recall@{}: {:.6f}", kNeighbours, recall_min);
        std::println("  total time:     {:.2f} ms", total_ms);
        std::println("  per query:      {:.2f} ms",
                     total_ms / static_cast<double>(queries.size()));

        if (mean_recall != 1.0) {
            std::println("\nFAIL: exact search did not reproduce the ground truth "
                         "(expected Recall@{} = 1.0)",
                         kNeighbours);
            return 1;
        }

        std::println("\nOK: Recall@{} = 1.0 baseline reproduced.", kNeighbours);
        return 0;
    } catch (const std::exception &e) {
        std::println("error: {}", e.what());
        std::println("hint: fetch the dataset first with scripts/fetch_siftsmall.ps1 "
                     "(or scripts/fetch_siftsmall.sh), or pass the data directory as "
                     "the first argument.");
        return 1;
    }
}
