#include <cstddef>
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
#endif

    // store the mapped address and size for use by the rest of MmapArena
    impl_->data = static_cast<std::byte *>(mapped);
    impl_->size = size;
}

} // namespace vecdb
