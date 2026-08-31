#pragma once

#include <cstdint>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <vecdb/vector_record.hpp>

namespace vecdb {

/// Bridge between the on-disk vector formats and the in-memory records.
///
/// The .fvecs/.ivecs layout repeats, once per vector: a 4-byte little-endian
/// int32 dimension, followed by that many 4-byte elements. .fvecs holds floats
/// (base and query vectors), .ivecs holds int32_t (ground-truth indices).
class VectorStoreIO {
  public:
    /// Reads an entire .fvecs/.ivecs file into records with sequential ids
    /// starting at 0, matching the order the vectors appear in the file.
    ///
    /// Throws std::runtime_error if the file cannot be opened, if a record is
    /// truncated, or if a dimension header is not positive. Reaching EOF on a
    /// record boundary terminates the read normally.
    template <typename T>
    static std::vector<VectorRecord<T>> read_vecs(const std::string &file_path) {
        static_assert(std::is_trivially_copyable_v<T>,
                      "read_vecs copies raw bytes into T, which must be trivially copyable");
        static_assert(sizeof(T) == 4, ".fvecs/.ivecs elements are 4 bytes wide");

        // The formats store little-endian values. Every platform this targets
        // is little-endian, so the bytes are read straight into memory rather
        // than byte-swapped; a big-endian port would need to swap here.
        std::ifstream in(file_path, std::ios::binary);
        if (!in) {
            throw std::runtime_error("VectorStoreIO: cannot open file: " + file_path);
        }

        std::vector<VectorRecord<T>> records;
        std::int32_t dim = 0;
        int next_id = 0;

        while (in.read(reinterpret_cast<char *>(&dim), sizeof(dim))) {
            if (dim <= 0) {
                throw std::runtime_error("VectorStoreIO: non-positive dimension " +
                                         std::to_string(dim) + " for vector " +
                                         std::to_string(next_id) + " in " + file_path);
            }

            VectorRecord<T> record;
            record.id = next_id++;
            record.vector.resize(static_cast<std::size_t>(dim));

            const std::streamsize payload = static_cast<std::streamsize>(sizeof(T)) * dim;
            in.read(reinterpret_cast<char *>(record.vector.data()), payload);
            if (in.gcount() != payload) {
                throw std::runtime_error("VectorStoreIO: truncated vector " +
                                         std::to_string(record.id) + " in " + file_path);
            }

            records.push_back(std::move(record));
        }

        // The loop exits on the first failed header read. A clean EOF reads
        // zero bytes; anything in between is a partially written header.
        if (in.gcount() != 0) {
            throw std::runtime_error("VectorStoreIO: truncated dimension header for vector " +
                                     std::to_string(next_id) + " in " + file_path);
        }
        if (in.bad()) {
            throw std::runtime_error("VectorStoreIO: I/O error while reading " + file_path);
        }

        return records;
    }
};

}  // namespace vecdb
