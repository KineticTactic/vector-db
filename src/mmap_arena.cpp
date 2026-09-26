#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <vecdb/mmap_arena.hpp>

#ifdef _WIN32
// Windows implementation
#else
// POSIX implementation for macOS/Linux
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace vecdb {

#ifdef _WIN32
// Windows implementation
#else
// POSIX implementation for macOS/Linux
struct MmapArena::Impl {
    // file descriptor for backing file
    int fd = -1;
    // starting address of memory mapped region
    std::byte *data = nullptr;
    // size of mapped region in bytes
    std::size_t size = 0;
    AccessMode mode; // needed as grow() is supposed to reject read only arenas acc to doc
};
#endif

MmapArena::MmapArena(const std::filesystem::path &path, std::size_t size, AccessMode mode) {
    // 0 sized mapping is invalid
    if (size == 0) {
        throw std::invalid_argument("Mapping size must be greater than zero");
    }

    // create the platform specific implementation object
    impl_ = std::make_unique<Impl>();

    // choose file access flags based on access mode
    int flags;

#ifdef _WIN32
    throw std::runtime_error("Not implemented");
#else
    impl_->mode = mode; // assign the moed
    // POSIX implementation for macOS/Linux

    if (mode == AccessMode::ReadOnly) {
        flags = O_RDONLY;
    } else {
        // open for reading/writing and create the file if it does not exist already
        flags = O_RDWR | O_CREAT;
    }

    // 0644 specifies permissions if a new file is created
    int fd = open(path.c_str(), flags, 0644);
    if (fd == -1) {
        throw std::runtime_error("Failed to open file");
    }

    impl_->fd = fd;

    // get info about the backing file including its size
    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        close(fd);
        throw std::runtime_error("Failed to get file size");
    }

    // enlarge the backing file if it is smaller than the requested mapping
    if (file_stat.st_size < static_cast<off_t>(size)) {
        if (ftruncate(fd, static_cast<off_t>(size)) == -1) {
            close(fd);
            throw std::runtime_error("Failed to resize file");
        }
    }

    // set memory protection according to the access mode used
    int protection;
    if (mode == AccessMode::ReadOnly) {
        protection = PROT_READ;
    } else {
        protection = PROT_READ | PROT_WRITE;
    }

    // map the backing file into the process's virtual memory
    void *mapped = mmap(nullptr, size, protection, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        close(fd);
        throw std::runtime_error("Failed to create memory mapping");
    }
    // store the mapped address and size for use by the rest of MmapArena
    impl_->data = static_cast<std::byte *>(mapped);
    impl_->size = size;
#endif
}

MmapArena::~MmapArena() {
#ifndef _WIN32
    if (impl_ == nullptr) {
        return;
    }

    if (impl_->data != nullptr) {
        munmap(impl_->data, impl_->size);
    }

    if (impl_->fd != -1) {
        close(impl_->fd);
    }
#endif
}

#ifndef _WIN32
std::span<const std::byte> MmapArena::data() const noexcept {
    return std::span<const std::byte>(impl_->data, impl_->size);
}

std::span<std::byte> MmapArena::mutable_data() {
    if (impl_->mode == AccessMode::ReadOnly) {
        throw std::logic_error("Cannot get mutable data for a read-only mapping");
    }
    return std::span<std::byte>(impl_->data, impl_->size);
}

std::size_t MmapArena::size() const noexcept { return impl_->size; }
#endif

void MmapArena::grow(std::size_t new_size) {
#ifdef _WIN32
    (void)new_size;
    throw std::runtime_error("Not implemented");
#else
    if (new_size == 0) {
        throw std::invalid_argument(
            "Mapping size must be greater than zero"); // why initialize khaali map son?
    }

    if (new_size <= impl_->size) {
        throw std::invalid_argument(
            "New mapping size must be greater than the current size"); // if no want to grow then
                                                                       // why call grow()
    }

    if (impl_->mode == AccessMode::ReadOnly) {
        throw std::logic_error("Cannot grow a read-only mapping"); // added the mode field in the
                                                                   // struct just to check for this
    }

    if (new_size > static_cast<std::size_t>(
                       std::numeric_limits<off_t>::max())) { // payload too large (elite ball)
        throw std::overflow_error("Mapping size is too large");
    }

    if (msync(impl_->data, impl_->size, MS_SYNC) == -1) {
        throw std::runtime_error("Failed to flush mapping before growth"); //
    }

    if (munmap(impl_->data, impl_->size) == -1) {
        throw std::runtime_error("Failed to unmap old mapping");
    }

    // previous mapping and all pointers into it are invalid now
    impl_->data = nullptr;
    impl_->size = 0;

    if (ftruncate(impl_->fd, static_cast<off_t>(new_size)) == -1) {
        throw std::runtime_error("Failed to resize backing file");
    }

    void *mapped = mmap(nullptr, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, impl_->fd, 0);
    if (mapped == MAP_FAILED) {
        throw std::runtime_error("Failed to create enlarged memory mapping");
    }

    impl_->data = static_cast<std::byte *>(mapped);
    impl_->size = new_size;
#endif
}

} // namespace vecdb
