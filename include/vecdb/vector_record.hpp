#pragma once

#include <any>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace vecdb {

/// Flexible key-value store attached to a vector record.
///
/// `std::any` lets a value hold any type ("filename" -> std::string,
/// "page_number" -> int) without the record committing to a fixed schema.
/// Declared for extensibility; Phase 0 never populates it.
using Metadata = std::unordered_map<std::string, std::any>;

/// One complete vector entry in the database.
///
/// Templated on the element type so the same abstraction covers float vectors
/// (.fvecs base/query data) and int32_t vectors (.ivecs ground-truth indices).
template <typename T>
struct VectorRecord {
    int id;
    std::vector<T> vector;
    Metadata metadata;

    std::size_t dimension() const { return vector.size(); }
};

}  // namespace vecdb
