#pragma once

#include<cstddef>
#include<vector>
#include<vecdb/vector_record.hpp>

namespace vecdb{

std::vector<int> flat_search(const std::vector<float>& query, const std::vector<VectorRecord<float>>& base, std::size_t k);

}