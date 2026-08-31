#pragma once

#include <cstddef>
#include <vector>

namespace vecdb {

/// Fraction of the true top-k neighbours that a search actually retrieved.
///
/// Compares the first k ids of `retrieved` against the first k of
/// `ground_truth` as sets: order is irrelevant, since two correct rankings can
/// still disagree on the order of exactly-tied distances.
///
/// Throws std::invalid_argument if k is 0 or either input holds fewer than k
/// ids.
double recall_at_k(const std::vector<int> &retrieved, const std::vector<int> &ground_truth,
                   std::size_t k);

}  // namespace vecdb
