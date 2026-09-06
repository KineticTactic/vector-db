#include <vecdb/vector_store_io.hpp>

#include <cstdint>
#include <fstream>
#include <stdexcept>

namespace vecdb {

template <typename T>
std::vector<VectorRecord<T>> VectorStoreIO::read_vecs(const std::string &file_path) {
    std::ifstream input(file_path, std::ios::binary);

    if (!input.is_open()) { // check if able to open
        throw std::runtime_error("Failed to open file: " + file_path);
    }

    std::vector<VectorRecord<T>> records;
    int id = 0;

    while (1) {
        std::int32_t dimension = 0;

        // reading the dimensoin
        // reinterpret_cast is used because read() requires a pointer to raw bytes- static_cast
        // cannot convert this typed pointer to char*.
        input.read(reinterpret_cast<char *>(&dimension), sizeof(dimension));

        if (input.gcount() == 0 && input.eof()) { // natural cause of death
            break;
        }
        if (input.gcount() != sizeof(dimension)) { // murder
            throw std::runtime_error("Truncated dimension");
        }
        if (dimension <= 0) { // suucide
            throw std::runtime_error("Invalid dimension");
        }

        std::vector<T> values(static_cast<std::size_t>(dimension));
        const auto value_bytes = static_cast<std::streamsize>(values.size() * sizeof(T));

        // reading the data
        input.read(reinterpret_cast<char *>(values.data()), value_bytes);

        if (input.gcount() != value_bytes) { // murder again lool
            throw std::runtime_error("Truncated vector data");
        }

        records.push_back(VectorRecord<T>{id, std::move(values), {}});

        ++id;
    }

    return records;
}

// Explicit instantiations for supported vector types.
template std::vector<vecdb::VectorRecord<float>>
vecdb::VectorStoreIO::read_vecs<float>(const std::string &);

template std::vector<vecdb::VectorRecord<int>>
vecdb::VectorStoreIO::read_vecs<int>(const std::string &);

} // namespace vecdb
