#include <atomic>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include <vecdb/mmap_arena.hpp>

#ifndef _WIN32
namespace {

class MmapArenaTest : public ::testing::Test {
  protected:
    void SetUp() override {
        static std::atomic<unsigned> next_id = 0;
        path_ = std::filesystem::temp_directory_path() /
                ("vecdb_mmap_arena_test_" + std::to_string(next_id++) + ".dat");
        std::filesystem::remove(path_);
    }

    void TearDown() override { std::filesystem::remove(path_); }

    std::filesystem::path path_;
};

TEST_F(MmapArenaTest, CreatesFileAndReportsRequestedSize) {
    constexpr std::size_t mapping_size = 4096;

    vecdb::MmapArena arena(path_, mapping_size, vecdb::AccessMode::ReadWrite);

    ASSERT_TRUE(std::filesystem::exists(path_));
    EXPECT_EQ(std::filesystem::file_size(path_), mapping_size);
    EXPECT_EQ(arena.size(), mapping_size);
    EXPECT_EQ(arena.data().size(), mapping_size);
    EXPECT_EQ(arena.mutable_data().size(), mapping_size);
}

TEST_F(MmapArenaTest, ReadsAndWritesThroughSpans) {
    vecdb::MmapArena arena(path_, 4096, vecdb::AccessMode::ReadWrite);

    auto writable = arena.mutable_data();
    writable[0] = std::byte{42};
    writable[4095] = std::byte{7};

    const auto &const_arena = arena;
    EXPECT_EQ(const_arena.data()[0], std::byte{42});
    EXPECT_EQ(const_arena.data()[4095], std::byte{7});
}

TEST_F(MmapArenaTest, OpensExistingFileAndReadsContents) {
    {
        std::ofstream output(path_, std::ios::binary);
        ASSERT_TRUE(output);
        const std::byte contents[] = {std::byte{11}, std::byte{22}, std::byte{33}};
        output.write(reinterpret_cast<const char *>(contents), sizeof(contents));
    }

    vecdb::MmapArena arena(path_, 3, vecdb::AccessMode::ReadOnly);

    EXPECT_EQ(arena.data()[0], std::byte{11});
    EXPECT_EQ(arena.data()[1], std::byte{22});
    EXPECT_EQ(arena.data()[2], std::byte{33});
}

TEST_F(MmapArenaTest, ReadOnlyArenaAllowsReadingButRejectsMutableAccess) {
    {
        vecdb::MmapArena arena(path_, 4096, vecdb::AccessMode::ReadWrite);
        arena.mutable_data()[0] = std::byte{42};
    }

    vecdb::MmapArena arena(path_, 4096, vecdb::AccessMode::ReadOnly);

    EXPECT_EQ(arena.data()[0], std::byte{42});
    EXPECT_THROW((void)arena.mutable_data(), std::logic_error);
}

TEST_F(MmapArenaTest, RejectsInvalidConstruction) {
    EXPECT_THROW((void)vecdb::MmapArena(path_, 0, vecdb::AccessMode::ReadWrite),
                 std::invalid_argument);

    const auto invalid_path = path_ / "not_a_file";
    EXPECT_THROW((void)vecdb::MmapArena(invalid_path, 4096, vecdb::AccessMode::ReadWrite),
                 std::runtime_error);
}

TEST_F(MmapArenaTest, GrowsAndPreservesExistingData) {
    constexpr std::size_t initial_size = 4096;
    constexpr std::size_t grown_size = 8192;

    vecdb::MmapArena arena(path_, initial_size, vecdb::AccessMode::ReadWrite);
    arena.mutable_data()[0] = std::byte{42};
    arena.mutable_data()[initial_size - 1] = std::byte{7};

    arena.grow(grown_size);

    EXPECT_EQ(arena.size(), grown_size);
    EXPECT_EQ(arena.data()[0], std::byte{42});
    EXPECT_EQ(arena.data()[initial_size - 1], std::byte{7});
    EXPECT_EQ(std::filesystem::file_size(path_), grown_size);
}

TEST_F(MmapArenaTest, NewRegionAfterGrowthIsAccessible) {
    constexpr std::size_t grown_size = 8192;

    vecdb::MmapArena arena(path_, 4096, vecdb::AccessMode::ReadWrite);
    arena.grow(grown_size);

    auto writable = arena.mutable_data();
    writable[grown_size - 1] = std::byte{99};

    EXPECT_EQ(arena.data()[grown_size - 1], std::byte{99});
}

TEST_F(MmapArenaTest, RejectsInvalidGrowth) {
    constexpr std::size_t current_size = 4096;
    vecdb::MmapArena arena(path_, current_size, vecdb::AccessMode::ReadWrite);

    EXPECT_THROW(arena.grow(0), std::invalid_argument);
    EXPECT_THROW(arena.grow(current_size), std::invalid_argument);
    EXPECT_THROW(arena.grow(current_size - 1), std::invalid_argument);
}

TEST_F(MmapArenaTest, RejectsGrowthForReadOnlyArena) {
    {
        vecdb::MmapArena writable(path_, 4096, vecdb::AccessMode::ReadWrite);
    }

    vecdb::MmapArena arena(path_, 4096, vecdb::AccessMode::ReadOnly);

    EXPECT_THROW(arena.grow(8192), std::logic_error);
}

TEST_F(MmapArenaTest, NewMappingContainsDataAfterGrowth) {
    vecdb::MmapArena arena(path_, 4096, vecdb::AccessMode::ReadWrite);
    arena.mutable_data()[128] = std::byte{55};

    arena.grow(8192);

    // The old span must not be used after grow(); data is checked through the
    // newly acquired mapping.
    EXPECT_EQ(arena.data()[128], std::byte{55});
}

} // namespace
#endif
