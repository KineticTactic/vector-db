# Phase 1.1

## Overview 
`MmapArena` class is an API to load a memory-mapped file and access it. It is the lowest level storage abstraction of the vector database. It can open existing files or create a new file of a given size. The file is loaded as a memory-mapped file using `mmap` which creates a VMA, and the OS automatically syncs this memory region with the file on disk. 

The class is responsible for the complete lifecycle of the mapping:

- opening or creating the backing file
- creating the memory mapping
- exposing the mapped bytes
- growing the mapped region
- flushing changes
- releasing the mapping and underlying file resources

The data is stored as a `std::byte` array. The size of the memory area can be changed with the `grow` method, however, using it will recreate the VMA and invalidate any previous pointers pointing to `data()`.

## Public API
```cpp

enum class AccessMode { ReadOnly, ReadWrite };

class MmapArena {
public:
    MmapArena(const std::filesystem::path& path, std::size_t size, AccessMode mode);

    ~MmapArena();

// REMOVE copy constructor and copy assignment
    MmapArena(const MmapArena&) = delete;
    MmapArena& operator=(const MmapArena&) = delete;

// EXPLICIT move constructor and move assignment
    MmapArena(MmapArena&& other) noexcept;
    MmapArena& operator=(MmapArena&& other) noexcept;

// for editing data
    std::byte* data() noexcept;
// for reading
    const std::byte* data() const noexcept;

// getter
    std::size_t size() const noexcept;

// WARNING: USING THIS WILL INVALIDATE ALL POINTERS?
    void grow(std::size_t new_size);

// flush changes to DISK
    void flush();

private:
    std::byte* data_;
    std::size_t size_;
};

```

## Implementation plan

### 1. Constructor and memory-map creation.

Create the files `src/mmap_arena.cpp` and `include/vecdb/mmap_arena.hpp` and create the basic class implementation (basically the constructor).

**The header should be platform-INDEPENDENT.**

In the src file, use macros to write platform specific code.
```cpp
#ifdef _WIN32
    // Windows
#elif defined(__APPLE__)
    // macOS
#elif defined(__linux__)
    // Linux
#else
    #error "Unsupported platform"
#endif
```

use **PImpl** design. https://en.cppreference.com/cpp/language/pimpl
store a 
```cpp
private:
    struct Impl;
    Impl* impl_; // USE UNIQJUE POINTER
```
in the header. Keep platform specific member variables in this struct (if required). 

If it gets too large we can split into separate impl files later.
```
src/storage/
├── mmap_arena.hpp
├── mmap_arena_posix.cpp
└── mmap_arena_windows.cpp
```

**For the constructor:**
1. Validate the requested size.
2. Open the existing file or create it if necessary (use appropriate read/write permissions.)
3. Ensure the backing file is large enough for the requested mapping.
4. Create the memory mapping with the requested access mode.
5. Store the resulting mapping and resource handles in `Impl`.

### 2. Data read/write and grow()

Implement the `data()` function (both const and non-const for read and write respectively). Should return a pointer to `std::byte`.

Implement `size()` function. Returns a `size_t`.

Implement the `grow()` function which should work as follows:

```
grow() 
│
├── validate new size 
├── flush old mapping 
├── munmap old mapping 
├── ftruncate file 
└── mmap new region
```

On POSIX this corresponds conceptually to:

```
msync()
munmap()
ftruncate()
mmap()
```

**Key decision:** This implementation would invalidate any previously made pointers to the data() area as we are creating a new region entirely. 

### 3. flush(), destructor and extensive testing

Implement `flush()`. It should immediately write changes to the memory region to disk.

Implement destructor, ensure `munmap` is called, and file pointers are closed (if any).
```
MmapArena::~MmapArena()
    │
    ├── release mapping
    │
    └── close file resource
```

The class follows RAII:

```
constructor → acquire resources
destructor  → release resources
```

Implement appropriate copy and move constructors. Copying is disabled because two `MmapArena` objects must not independently own the same OS resources. Moving is supported so ownership can be transferred between objects.

## Testing
Test file:
```
test/storage/mmap_arena_test.cpp
```

### Construction tests

1. Creates a new file

```
Create arena
→ backing file does not exist
→ construct arena
→ file exists
→ file has requested size
```

2. Opens an existing file

```
Create file
→ write known contents
→ construct arena
→ verify contents are accessible
```

3. Invalid construction

Test cases such as:

```
zero-sized mapping
invalid path
file cannot be opened
mapping fails
```

### Access-mode tests

1. Read/write

```
Create ReadWrite arena
→ write through data()
→ read value back
→ flush
→ reopen
→ verify value persisted
```

2. Read-only

```
Open ReadOnly arena
→ verify existing data can be read
→ verify mutable access is rejected
```

The exact mechanism for rejecting mutable access should be determined during implementation.

### Data access tests

Verify:

```
data() returns valid mapped storage
size() returns expected size
writes are visible through the mapping
```

Example:

```
auto* data = arena.data();

data[0] = std::byte{42};

EXPECT_EQ(data[0], std::byte{42});
```

### Persistence Test
1. Create arena
2. Write known bytes
3. flush()
4. Destroy arena
5. Create new arena using same file
6. Read bytes
7. Verify data persisted

### Data survives growth

```
Create 4 KB arena
→ write data
→ grow to 8 KB
→ verify old data still exists
```

### New region is accessible

```
grow from 4 KB → 8 KB
→ write inside newly available region
→ read it back
```

### Invalid growth is rejected

```
grow(current_size)
grow(smaller_size)
```

Both should be rejected according to the `grow()` contract.

### Pointer invalidation

The test should verify the **new mapping contains the old data**, rather than dereferencing an old pointer after `grow()`.

```
old pointer
grow()
old pointer must not be used
new data() pointer
verify preserved contents
```

### Move-semantics tests

Test:

```
move construction
move assignment
```

and verify:

```
destination owns the mapping
source no longer owns the mapping
resources are released exactly once
```

### Final invariants

The implementation should maintain these invariants:

1. A valid MmapArena owns exactly one mapping.
2. The mapping size equals size().
3. Only MmapArena performs platform-specific mapping operations.
4. Copy construction and copy assignment are disabled.
5. Move operations transfer ownership.
6. grow() invalidates all previous data() pointers.
7. ReadOnly arenas cannot be modified.
8. grow() is only valid for ReadWrite arenas.
9. flush() is the explicit persistence operation.
10. Destruction releases the mapping and underlying file resources.
