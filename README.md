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

- CMake, [how to setup a library-application architecture](https://cmake.org/cmake/help/latest/guide/tutorial/Getting%20Started%20with%20CMake.html)
- GoogleTest [setup](https://google.github.io/googletest/quickstart-cmake.html), [samples](https://google.github.io/googletest/samples.html)
- Commit messages, [conventional commits](https://www.conventionalcommits.org/en/v1.0.0/)
