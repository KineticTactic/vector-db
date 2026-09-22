# vector-db

[![CI](https://github.com/KineticTactic/vector-db/actions/workflows/ci.yml/badge.svg)](https://github.com/KineticTactic/vector-db/actions/workflows/ci.yml)
[![Format](https://github.com/KineticTactic/vector-db/actions/workflows/format.yml/badge.svg)](https://github.com/KineticTactic/vector-db/actions/workflows/format.yml)
[![Sanitizers](https://github.com/KineticTactic/vector-db/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/KineticTactic/vector-db/actions/workflows/sanitizers.yml)

`include` contains all the library header files. `src` contains the library source code. `test` contains GoogleTest test files. `example` contains executables which use the library.

If you add new cpp/hpp files then add them in `CMakeLists.txt` also.

## Features

Phase 0: [Design Doc](./doc/phase0.md)

- `VectorRecord` and `Metadata` structs.
- `VectorStoreIO` class for reading `fvec/ivec` file.
- Brute-force k-NN search with 100% Recall.

## Build

```
cmake -S . -B build     # Generate the build files in build/ directory
cmake --build build     # Build library, tests, examples
```

## Run examples

```
./build/flat_search_recall  # Test Recall@100 for sift_small dataset
```

## Run tests

```
./build/vecdb_tests
```

## Reading Material

### 1. General Project Setup
- CMake, [how to setup a library-application architecture](https://cmake.org/cmake/help/latest/guide/tutorial/Getting%20Started%20with%20CMake.html)
- GoogleTest [setup](https://google.github.io/googletest/quickstart-cmake.html), [samples](https://google.github.io/googletest/samples.html)
- Commit messages, [conventional commits](https://www.conventionalcommits.org/en/v1.0.0/)

### 2. Virtual Memory, Memory-mapped files

1. [OS Memory Mapping - Coding Club](https://codingclub.in/blog/os-memory-mapping#5-vmas-and-memory-mapped-files)
2. BitLemon
	1. [Virtual Memory Explained (including Paging)](https://youtu.be/fGP6VHxqkIM)
	2. [Page Tables and MMU: How Virtual Memory Actually Works Behind the Scenes](https://www.youtube.com/watch?v=B6tJxvYBNrU)
3. Jacob Sorber
	1. [How to Map Files into Memory in C (mmap, memory mapped file io)](https://www.youtube.com/watch?v=m7E9piHcfr4)
	2. [How processes get more memory. (mmap, brk)](https://www.youtube.com/watch?v=XV5sRaSVtXQ)
4. Dave's Garage
	1. [Malloc is NOT Magic: Let's Build it to Learn What's Inside!](https://www.youtube.com/watch?v=mYBxnojY-JA)
