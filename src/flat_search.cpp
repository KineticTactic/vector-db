#include <vecdb/flat_search.hpp>

#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <vecdb/distance.hpp>

namespace vecdb {

std::vector<int> flat_search(const std::vector<float> &query,
                             const std::vector<VectorRecord<float>> &base, std::size_t k) {
    if (k == 0) {
        throw std::invalid_argument("flat_search: k must be greater than 0");
    }
    if (k > base.size()) {
        throw std::invalid_argument("flat_search: k (" + std::to_string(k) +
                                    ") exceeds dataset size (" + std::to_string(base.size()) + ")");
    }

    // std::priority_queue is a max-heap by default, so heap.top() is the worst
    // candidate held so far - exactly what a bounded top-k scan needs to evict.
    // Comparing std::pair orders on distance first and breaks ties on id.
    std::priority_queue<std::pair<float, int>> heap;

    for (const VectorRecord<float> &record : base) {
        if (record.dimension() != query.size()) {
            throw std::invalid_argument("flat_search: record " + std::to_string(record.id) +
                                        " has dimension " + std::to_string(record.dimension()) +
                                        ", query has " + std::to_string(query.size()));
        }

        const float distance = squared_l2(query, record.vector);

        if (heap.size() < k) {
            heap.emplace(distance, record.id);
        } else if (distance < heap.top().first) {
            heap.pop();
            heap.emplace(distance, record.id);
        }
    }

    // Popping a max-heap yields the largest distance first, so fill the result
    // back to front to get ascending distance order.
    std::vector<int> ids(k);
    for (std::size_t i = k; i-- > 0;) {
        ids[i] = heap.top().second;
        heap.pop();
    }
    return ids;
}

}  // namespace vecdb
