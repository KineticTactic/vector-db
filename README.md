# vector-db

Currently it is structured as a library-executable architecture. `vecdb` is the name of the library being compiled. 

If you add new cpp/hpp files then add them in `CMakeLists.txt` also.

`include` contains all the library header files. `src` contains the library source code. `test` contains GoogleTest test files. `example` contains executables which use the library.

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
