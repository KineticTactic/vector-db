#pragma once

#include <cstddef>
#include <vector>

#include <vecdb/vector_record.hpp>

namespace vecdb {

/// Exact brute-force k-nearest-neighbour search.
///
/// Scores the query against every record in `base` using squared L2 distance
/// and returns the ids of the k closest, ordered from smallest distance to
/// largest. This is the correctness baseline that approximate indexes in later
/// phases are measured against.
///
/// Throws std::invalid_argument if k is 0 or larger than the dataset, or if any
/// base record's dimension differs from the query's.
std::vector<int> flat_search(const std::vector<float> &query,
                             const std::vector<VectorRecord<float>> &base, std::size_t k);

}  // namespace vecdb
