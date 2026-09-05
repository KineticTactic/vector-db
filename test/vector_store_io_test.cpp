#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <vecdb/vector_store_io.hpp>

void writeFloatVector(const std::string &filename, const std::vector<float> &values) {
    std::ofstream output(filename, std::ios::binary);

    std::int32_t dimension = static_cast<std::int32_t>(values.size());

    output.write(reinterpret_cast<const char *>(&dimension), sizeof(dimension));

    output.write(reinterpret_cast<const char *>(values.data()), values.size() * sizeof(float));
}

// Test that a single float vector is read correctly from a binary file.
TEST(VectorStoreIOTest, ReadsSingleFloatVector) {
    const std::string filename = "test_single.fvecs";

    // Create a test file containing one 3-dimensional vector.
    writeFloatVector(filename, {1.0f, 2.0f, 3.0f});

    // Load the vector using VectorStoreIO.
    const auto records = vecdb::VectorStoreIO::read_vecs<float>(filename);

    // Verify that exactly one vector was loaded.
    ASSERT_EQ(records.size(), 1u);

    // Verify the vector's ID, dimension, and values.
    EXPECT_EQ(records[0].id, 0);
    EXPECT_EQ(records[0].dimension(), 3u);
    EXPECT_EQ(records[0].vector[0], 1.0f);
    EXPECT_EQ(records[0].vector[1], 2.0f);
    EXPECT_EQ(records[0].vector[2], 3.0f);

    // Remove the temporary test file.
    std::remove(filename.c_str());
}

// Test that multiple vectors stored sequentially in one file
// are loaded correctly and assigned sequential IDs.
TEST(VectorStoreIOTest, ReadsMultipleFloatVectors) {
    const std::string filename = "test_multiple.fvecs";

    // Create a binary file containing two 3-dimensional vectors.
    std::ofstream output(filename, std::ios::binary);

    std::int32_t dimension = 3;

    std::vector<float> first = {1.0f, 2.0f, 3.0f};
    std::vector<float> second = {4.0f, 5.0f, 6.0f};

    // Write the first vector: dimension followed by its values.
    output.write(reinterpret_cast<const char *>(&dimension), sizeof(dimension));
    output.write(reinterpret_cast<const char *>(first.data()), first.size() * sizeof(float));

    // Write the second vector immediately after the first.
    output.write(reinterpret_cast<const char *>(&dimension), sizeof(dimension));
    output.write(reinterpret_cast<const char *>(second.data()), second.size() * sizeof(float));

    output.close();

    // Load the vectors using the VectorStoreIO implementation.
    const auto records = vecdb::VectorStoreIO::read_vecs<float>(filename);

    // Two vectors should have been loaded.
    ASSERT_EQ(records.size(), 2u);

    // The first vector should have ID 0 and the expected values.
    EXPECT_EQ(records[0].id, 0);
    EXPECT_EQ(records[0].vector, first);

    // The second vector should have ID 1 and the expected values.
    EXPECT_EQ(records[1].id, 1);
    EXPECT_EQ(records[1].vector, second);

    // Remove the temporary test file.
    std::remove(filename.c_str());
}

// Test that attempting to read a file that cannot be opened
// throws a runtime_error.
TEST(VectorStoreIOTest, ThrowsWhenFileCannotBeOpened) {
    const std::string filename = "does_not_exist.fvecs";

    EXPECT_THROW(vecdb::VectorStoreIO::read_vecs<float>(filename), std::runtime_error);
}

// Test that a file containing an incomplete dimension
// throws a runtime_error.
TEST(VectorStoreIOTest, ThrowsOnTruncatedDimension) {
    const std::string filename = "test_truncated_dimension.fvecs";

    // Create a file containing only part of the 4-byte dimension.
    std::ofstream output(filename, std::ios::binary);

    std::int16_t partial_dimension = 3;

    output.write(reinterpret_cast<const char *>(&partial_dimension), sizeof(partial_dimension));

    output.close();

    // Reading the incomplete dimension should throw an error.
    EXPECT_THROW(vecdb::VectorStoreIO::read_vecs<float>(filename), std::runtime_error);

    // Remove the temporary test file.
    std::remove(filename.c_str());
}

// Test that a vector with fewer values than its declared dimension
// throws a runtime_error.
TEST(VectorStoreIOTest, ThrowsOnTruncatedVectorData) {
    const std::string filename = "test_truncated_data.fvecs";

    // Declare a 3-dimensional vector but write only 2 values.
    std::ofstream output(filename, std::ios::binary);

    std::int32_t dimension = 3;
    std::vector<float> values = {1.0f, 2.0f};

    output.write(reinterpret_cast<const char *>(&dimension), sizeof(dimension));

    output.write(reinterpret_cast<const char *>(values.data()), values.size() * sizeof(float));

    output.close();

    // Reading the incomplete vector should throw an error.
    EXPECT_THROW(vecdb::VectorStoreIO::read_vecs<float>(filename), std::runtime_error);

    // Remove the temporary test file.
    std::remove(filename.c_str());
}

// Test that a vector with a dimension of zero
// throws a runtime_error.
TEST(VectorStoreIOTest, ThrowsOnZeroDimension) {
    const std::string filename = "test_zero_dimension.fvecs";

    // Create a file containing an invalid zero dimension.
    std::ofstream output(filename, std::ios::binary);

    std::int32_t dimension = 0;

    output.write(reinterpret_cast<const char *>(&dimension), sizeof(dimension));

    output.close();

    // Reading a vector with zero dimension should throw an error.
    EXPECT_THROW(vecdb::VectorStoreIO::read_vecs<float>(filename), std::runtime_error);

    // Remove the temporary test file.
    std::remove(filename.c_str());
}

// Test that a vector with a negative dimension
// throws a runtime_error.
TEST(VectorStoreIOTest, ThrowsOnNegativeDimension) {
    const std::string filename = "test_negative_dimension.fvecs";

    // Create a file containing an invalid negative dimension.
    std::ofstream output(filename, std::ios::binary);

    std::int32_t dimension = -3;

    output.write(reinterpret_cast<const char *>(&dimension), sizeof(dimension));

    output.close();

    // Reading a vector with a negative dimension should throw an error.
    EXPECT_THROW(vecdb::VectorStoreIO::read_vecs<float>(filename), std::runtime_error);

    // Remove the temporary test file.
    std::remove(filename.c_str());
}

// Test that integer vectors can be loaded correctly,
// as required for .ivecs ground-truth data.
TEST(VectorStoreIOTest, ReadsInt32Vectors) {
    const std::string filename = "test_int32.ivecs";

    // Create a binary file containing one integer vector.
    std::ofstream output(filename, std::ios::binary);

    std::int32_t dimension = 3;
    std::vector<std::int32_t> values = {10, 20, 30};

    output.write(reinterpret_cast<const char *>(&dimension), sizeof(dimension));

    output.write(reinterpret_cast<const char *>(values.data()),
                 values.size() * sizeof(std::int32_t));

    output.close();

    // Load the vector as int32_t values.
    const auto records = vecdb::VectorStoreIO::read_vecs<std::int32_t>(filename);

    // Verify that the vector was loaded correctly.
    ASSERT_EQ(records.size(), 1u);
    EXPECT_EQ(records[0].id, 0);
    EXPECT_EQ(records[0].dimension(), 3u);
    EXPECT_EQ(records[0].vector, values);

    // Remove the temporary test file.
    std::remove(filename.c_str());
}