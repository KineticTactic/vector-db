# vector-db

Currently it is structured as a library-executable architecture. `vecdb` is the name of the library being compiled. `vector-db-main` is the executable generated from `main.cpp` which links with the library. 

If you add new cpp/hpp files then add them in `CMakeLists.txt` also.

`include` contains all the library header files. `src` contains the library source code. `test` contains GoogleTest test files.

## Build

```
cmake -S . -B build     # Generate the build files in build/ directory
cmake --build build     # Build 

./build/vector-db-main  # Run the built executable

./build/hello_test      # Run the tests
```
