#pragma once

#include <any>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace vecdb {

/// Flexible key-value payload attached to a vector record.
///
/// The string keys name individual fields and `std::any` lets each value carry
/// its own type ("filename" -> std::string, "page_number" -> int), so a record
/// is never tied to a fixed metadata schema. Wrapping the map in a struct
/// instead of aliasing it keeps `Metadata` a distinct type that later phases
/// can grow (typed accessors, filtering, serialization) without changing the
/// members of `VectorRecord`. Declared for extensibility; Phase 0 never
/// populates it.
struct Metadata {
    std::unordered_map<std::string, std::any> fields;
};

/// One complete vector entry in the database: an internal identifier, the
/// vector values themselves, and any metadata attached to them.
///
/// Templated on the element type so the same abstraction covers float vectors
/// (.fvecs base and query data) and int32_t vectors (.ivecs ground-truth
/// indices). Ids are sequential from 0 in dataset order.
template <typename T>
struct VectorRecord {
    int id;
    std::vector<T> vector;
    Metadata metadata;

    std::size_t dimension() const { return vector.size(); }
};

}  // namespace vecdb
