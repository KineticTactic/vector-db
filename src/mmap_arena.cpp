#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <vecdb/mmap_arena.hpp>

#ifdef _WIN32
// Windows implementation
#include <fileapi.h>
#include <io.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <vector>
#include <windows.h>

std::string windows_error_message(DWORD error) {
    wchar_t *buffer = nullptr;

    DWORD size = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, error, 0, reinterpret_cast<wchar_t *>(&buffer), 0, nullptr);

    if (size == 0) {
        return "Unknown Windows error: " + std::to_string(error);
    }

    std::wstring message(buffer, size);
    LocalFree(buffer);

    // Simple conversion for an ASCII-ish error message.
    return std::string(message.begin(), message.end());
}

[[noreturn]] void throw_windows_error(const char *operation) {
    DWORD error = GetLastError();

    throw std::runtime_error(std::string(operation) + " failed with Windows error " +
                             std::to_string(error));
}
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
struct MmapArena::Impl {
    HANDLE file;
    HANDLE mapping;
    std::size_t size = 0;
    std::byte *data = nullptr;
    AccessMode mode;
};
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
    const DWORD access =
        (mode == AccessMode::ReadOnly) ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);
    const DWORD protection = mode == AccessMode::ReadOnly ? PAGE_READONLY : PAGE_READWRITE;

    const std::uint64_t mapping_size = static_cast<std::uint64_t>(size);

    impl_->file = CreateFileW(path.c_str(), access, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    /// TODO: impl_->file check

    impl_->mapping =
        CreateFileMappingW(impl_->file, nullptr, protection, static_cast<DWORD>(mapping_size >> 32),
                           static_cast<DWORD>(mapping_size & 0xFFFFFFFF), nullptr);

    if (impl_->mapping == nullptr) {
        DWORD error = GetLastError();
        throw std::runtime_error("CreateFileMappingW failed: " + windows_error_message(error));
    }

    const DWORD view_access = mode == AccessMode::ReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE;
    impl_->data =
        static_cast<std::byte *>(MapViewOfFile(impl_->mapping,           // hFileMappingObject
                                               view_access,              // dwDesiredAccess
                                               0,                        // dwFileOffsetHigh
                                               0,                        // dwFileOffsetLow
                                               static_cast<SIZE_T>(size) // dwNumberOfBytesToMap
                                               ));

    if (impl_->data == nullptr) {
        DWORD error = GetLastError();
        throw std::runtime_error("MapViewOfFile failed: " + windows_error_message(error));
    }
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
#ifdef _WIN32
    if (impl_->data != nullptr)
        UnmapViewOfFile(impl_->data);

    if (impl_->mapping != nullptr)
        CloseHandle(impl_->mapping);

    if (impl_->file != INVALID_HANDLE_VALUE)
        CloseHandle(impl_->file);
#else
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

void MmapArena::grow(std::size_t new_size) {
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
#ifdef _WIN32
    // 1. Flush current mapping.
    if (!FlushViewOfFile(impl_->data, static_cast<SIZE_T>(impl_->size))) {
        throw_windows_error("FlushViewOfFile");
    }

    // Optional but useful if flush() is supposed to provide stronger
    // persistence semantics.
    if (!FlushFileBuffers(impl_->file)) {
        throw_windows_error("FlushFileBuffers");
    }

    // 2. Unmap old view.
    if (!UnmapViewOfFile(impl_->data)) {
        throw_windows_error("UnmapViewOfFile");
    }

    impl_->data = nullptr;

    // 3. Close old mapping object.
    if (!CloseHandle(impl_->mapping)) {
        impl_->mapping = nullptr;
        throw_windows_error("CloseHandle");
    }

    impl_->mapping = nullptr;

    // 4. Resize backing file.
    LARGE_INTEGER file_size{};
    file_size.QuadPart = static_cast<LONGLONG>(new_size);

    if (!SetFilePointerEx(impl_->file, file_size, nullptr, FILE_BEGIN)) {
        throw_windows_error("SetFilePointerEx");
    }

    if (!SetEndOfFile(impl_->file)) {
        throw_windows_error("SetEndOfFile");
    }

    // 5. Create a new mapping object with the new size.
    const std::uint64_t mapping_size = static_cast<std::uint64_t>(new_size);

    impl_->mapping = CreateFileMappingW(impl_->file, nullptr, PAGE_READWRITE,
                                        static_cast<DWORD>(mapping_size >> 32),
                                        static_cast<DWORD>(mapping_size & 0xFFFFFFFFu), nullptr);

    if (impl_->mapping == nullptr) {
        throw_windows_error("CreateFileMappingW");
    }

    // 6. Map the new view.
    void *mapped =
        MapViewOfFile(impl_->mapping, FILE_MAP_WRITE, 0, 0, static_cast<SIZE_T>(new_size));

    if (mapped == nullptr) {
        CloseHandle(impl_->mapping);
        impl_->mapping = nullptr;
        throw_windows_error("MapViewOfFile");
    }

    impl_->data = static_cast<std::byte *>(mapped);
    impl_->size = new_size;
#else
    if (new_size > static_cast<std::size_t>(
                       std::numeric_limits<off_t>::max())) { // payload too large (elite ball)
        throw std::overflow_error("Mapping size is too large");
    }

    /// TODO: Replace with flush()
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

void MmapArena::flush() {
    if (impl_->mode == AccessMode::ReadOnly) {
        throw std::logic_error("Cannot grow a read-only mapping");
    }
#ifdef _WIN32
    // 1. Flush current mapping.
    if (!FlushViewOfFile(impl_->data, static_cast<SIZE_T>(impl_->size))) {
        throw_windows_error("FlushViewOfFile");
    }

    // Optional but useful if flush() is supposed to provide stronger
    // persistence semantics.
    if (!FlushFileBuffers(impl_->file)) {
        throw_windows_error("FlushFileBuffers");
    }

#else
    if (msync(impl_->data, impl_->size, MS_SYNC) == -1) {
        throw std::runtime_error("Could not flush file!");
    }
#endif
}

} // namespace vecdb
