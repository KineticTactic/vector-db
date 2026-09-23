#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>

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
    MmapArena(MmapArena &&other) noexcept;
    MmapArena &operator=(MmapArena &&other) noexcept;

    // for reading and editing data
    std::byte *data() noexcept;
    // for reading only
    const std::byte *data() const noexcept;

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