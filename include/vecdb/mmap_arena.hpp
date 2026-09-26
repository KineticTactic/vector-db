#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <span>

namespace vecdb {

enum class AccessMode { ReadOnly, ReadWrite };

class MmapArena {
  public:
    MmapArena(const std::filesystem::path &path, std::size_t size, AccessMode mode);

    ~MmapArena();

    // remove copy constructor and copy assignment
    MmapArena(const MmapArena &) = delete;
    MmapArena &operator=(const MmapArena &) = delete;

    // explicit move constructor and move assignment
    MmapArena(MmapArena &&other) noexcept = default;
    MmapArena &operator=(MmapArena &&other) noexcept = default;

    // for reading only
    std::span<const std::byte> data() const noexcept;
    // for editing data
    std::span<std::byte> mutable_data();

    // return size of the mapped region
    std::size_t size() const noexcept;

    // grow the mapped region- existing pointers may be invalidated
    void grow(std::size_t new_size);

    // flush changes to backing file
    void flush();

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace vecdb
