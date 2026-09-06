#pragma once

#include <cstddef>
#include <vecdb/vector_record.hpp>
#include <vector>

namespace vecdb {

std::vector<int> flat_search(const std::vector<float> &query,
                             const std::vector<VectorRecord<float>> &base, std::size_t k);

}