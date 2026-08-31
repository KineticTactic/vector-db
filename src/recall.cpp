#include <vecdb/recall.hpp>

#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace vecdb {

double recall_at_k(const std::vector<int> &retrieved, const std::vector<int> &ground_truth,
                   std::size_t k) {
    if (k == 0) {
        throw std::invalid_argument("recall_at_k: k must be greater than 0");
    }
    if (retrieved.size() < k || ground_truth.size() < k) {
        throw std::invalid_argument(
            "recall_at_k: need at least k=" + std::to_string(k) + " ids, got " +
            std::to_string(retrieved.size()) + " retrieved and " +
            std::to_string(ground_truth.size()) + " ground-truth");
    }

    const std::unordered_set<int> expected(ground_truth.begin(), ground_truth.begin() + k);

    std::size_t hits = 0;
    for (std::size_t i = 0; i < k; ++i) {
        if (expected.count(retrieved[i]) != 0) {
            ++hits;
        }
    }

    return static_cast<double>(hits) / static_cast<double>(k);
}

}  // namespace vecdb
