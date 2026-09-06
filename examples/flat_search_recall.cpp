#include <iostream>
#include <unordered_set>

#include <vecdb/distance.hpp>
#include <vecdb/flat_search.hpp>
#include <vecdb/vector_store_io.hpp>

int main() {
    const std::string path = std::string(DATA_DIR) + "/siftsmall/";

    const auto base = vecdb::VectorStoreIO::read_vecs<float>(path + "base.fvecs");
    const auto queries = vecdb::VectorStoreIO::read_vecs<float>(path + "query.fvecs");
    const auto truth = vecdb::VectorStoreIO::read_vecs<int>(path + "groundtruth.ivecs");

    // Our dataset contains 100 nearest vectors for each query
    int k = 100;
    double total_recall = 0.0;

    for (size_t i = 0; i < queries.size(); i++) {
        const auto &query_vector = queries[i].vector;
        const auto &ground_truth_ids = truth[i].vector;

        const auto calculated_ids = vecdb::flat_search(query_vector, base, k);

        // Put calculated IDs into a set for O(1) lookup.
        std::unordered_set<int> calculated_id_set(calculated_ids.begin(), calculated_ids.end());

        int retrieved = 0;

        for (int id : ground_truth_ids) {
            if (calculated_id_set.contains(id)) {
                ++retrieved;
            }
        }

        const double recall = static_cast<double>(retrieved) / k;

        total_recall += recall;

        std::cout << "Query " << i << ": Recall@" << k << " = " << recall << '\n';
    }

    const double average_recall = total_recall / queries.size();

    std::cout << "\nAverage Recall@" << k << ": " << average_recall << '\n';

    return 0;
}
