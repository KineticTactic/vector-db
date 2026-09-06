#include <iostream>

#include <vecdb/flat_search.hpp>
#include <vecdb/vector_store_io.hpp>

int main() {
    const std::string path = std::string(DATA_DIR) + "/siftsmall/";

    const auto base = vecdb::VectorStoreIO::read_vecs<float>(path + "base.fvecs");
    const auto query = vecdb::VectorStoreIO::read_vecs<float>(path + "query.fvecs");
    const auto truth = vecdb::VectorStoreIO::read_vecs<int>(path + "groundtruth.ivecs");

    // Our dataset contains 100 nearest vectors for each query
    int k = 100;
    int correct = 0;

    for (size_t i = 0; i < query.size(); i++) {
        std::vector<int> nearest = vecdb::flat_search(query[i].vector, base, k);

        if (nearest == truth[i].vector) {
            std::cout << "MATCHING!!\n";
            correct++;
            continue;
        }

        for (int j = 0; j < 100; j++) {
            if (nearest[j] != truth[i].vector[j]) {
                std::cout << "MISMATCH AT " << j << "th index, found: " << nearest[j]
                          << ", truth: " << truth[i].vector[j] << "\n";
            }
        }
    }

    float recall = (float)correct / query.size();
    std::cout << "RECALL: " << recall << "\n";

    return 0;
}
