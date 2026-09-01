#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <vecdb/vector_store_io.hpp>

namespace {

/// Writes a .fvecs/.ivecs file: each vector is a 4-byte dimension followed by
/// its elements. Kept independent of the loader so the tests exercise the
/// format, not the loader's own idea of it.
template <typename T>
void write_vecs(const std::filesystem::path &path, const std::vector<std::vector<T>> &vectors) {
    std::ofstream out(path, std::ios::binary);
    ASSERT_TRUE(out) << "cannot create " << path.string();

    for (const std::vector<T> &vec : vectors) {
        const std::int32_t dim = static_cast<std::int32_t>(vec.size());
        out.write(reinterpret_cast<const char *>(&dim), sizeof(dim));
        out.write(reinterpret_cast<const char *>(vec.data()),
                  static_cast<std::streamsize>(sizeof(T) * vec.size()));
    }
}

/// Rewrites a file keeping only its first `bytes` bytes.
void truncate_file(const std::filesystem::path &path, std::uintmax_t bytes) {
    std::vector<char> buffer(static_cast<std::size_t>(bytes));
    {
        std::ifstream in(path, std::ios::binary);
        in.read(buffer.data(), static_cast<std::streamsize>(bytes));
    }
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(buffer.data(), static_cast<std::streamsize>(bytes));
}

class VectorStoreIOTest : public ::testing::Test {
  protected:
    void SetUp() override {
        dir_ = std::filesystem::temp_directory_path() /
               ("vecdb_io_test_" + std::to_string(
                                       ::testing::UnitTest::GetInstance()->random_seed()) +
                "_" + ::testing::UnitTest::GetInstance()->current_test_info()->name());
        std::filesystem::remove_all(dir_);
        std::filesystem::create_directories(dir_);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(dir_, ec);
    }

    std::filesystem::path path_for(const std::string &name) const { return dir_ / name; }

    std::filesystem::path dir_;
};

TEST_F(VectorStoreIOTest, ReadsFvecsWithSequentialIds) {
    const std::vector<std::vector<float>> data = {
        {1.0f, 2.0f, 3.0f}, {-1.5f, 0.0f, 2.25f}, {10.0f, 20.0f, 30.0f}};
    const auto path = path_for("base.fvecs");
    write_vecs(path, data);

    const auto records = vecdb::VectorStoreIO::read_vecs<float>(path.string());

    ASSERT_EQ(records.size(), data.size());
    for (std::size_t i = 0; i < data.size(); ++i) {
        EXPECT_EQ(records[i].id, static_cast<int>(i));
        EXPECT_EQ(records[i].dimension(), 3u);
        EXPECT_EQ(records[i].vector, data[i]);
        EXPECT_TRUE(records[i].metadata.fields.empty());
    }
}

TEST_F(VectorStoreIOTest, ReadsIvecsAsInt32) {
    const std::vector<std::vector<std::int32_t>> data = {{7, 8, 9, 10}, {0, -1, 2147483647, -5}};
    const auto path = path_for("groundtruth.ivecs");
    write_vecs(path, data);

    const auto records = vecdb::VectorStoreIO::read_vecs<std::int32_t>(path.string());

    ASSERT_EQ(records.size(), 2u);
    EXPECT_EQ(records[0].vector, data[0]);
    EXPECT_EQ(records[1].vector, data[1]);
    EXPECT_EQ(records[1].id, 1);
}

TEST_F(VectorStoreIOTest, EmptyFileYieldsNoRecords) {
    const auto path = path_for("empty.fvecs");
    { std::ofstream out(path, std::ios::binary); }

    EXPECT_TRUE(vecdb::VectorStoreIO::read_vecs<float>(path.string()).empty());
}

TEST_F(VectorStoreIOTest, MissingFileThrowsRuntimeError) {
    EXPECT_THROW(vecdb::VectorStoreIO::read_vecs<float>(path_for("nope.fvecs").string()),
                 std::runtime_error);
}

TEST_F(VectorStoreIOTest, TruncatedPayloadThrows) {
    const auto path = path_for("cut_payload.fvecs");
    write_vecs<float>(path, {{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}});

    // Keep the first full record plus the second record's header and one float.
    truncate_file(path, 16 + 4 + 4);

    EXPECT_THROW(vecdb::VectorStoreIO::read_vecs<float>(path.string()), std::runtime_error);
}

TEST_F(VectorStoreIOTest, TruncatedDimensionHeaderThrows) {
    const auto path = path_for("cut_header.fvecs");
    write_vecs<float>(path, {{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}});

    // Keep the first full record plus two of the second header's four bytes.
    truncate_file(path, 16 + 2);

    EXPECT_THROW(vecdb::VectorStoreIO::read_vecs<float>(path.string()), std::runtime_error);
}

TEST_F(VectorStoreIOTest, NonPositiveDimensionThrows) {
    const auto path = path_for("bad_dim.fvecs");
    {
        std::ofstream out(path, std::ios::binary);
        const std::int32_t dim = 0;
        out.write(reinterpret_cast<const char *>(&dim), sizeof(dim));
    }

    EXPECT_THROW(vecdb::VectorStoreIO::read_vecs<float>(path.string()), std::runtime_error);
}

}  // namespace
