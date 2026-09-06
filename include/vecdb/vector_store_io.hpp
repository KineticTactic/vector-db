#pragma once

#include <string>
#include <vector>

#include <vecdb/vector_record.hpp>

namespace vecdb {

// Reads binary .fvecs and .ivecs records into typed VectorRecord objects.
class VectorStoreIO {
  public:
    template <typename T>
    static std::vector<VectorRecord<T>> read_vecs(const std::string &file_path);
};

} // namespace vecdb
