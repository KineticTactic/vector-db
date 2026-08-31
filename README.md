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
./build/vecdb_tests     # Run the Phase 0 tests
```

Multi-config generators (Visual Studio) put the binaries one level deeper, e.g.
`./build/Debug/vector-db-main.exe`, and `ctest` needs the config:
`ctest --test-dir build -C Debug`.

## Dataset

Phase 0 is validated against [siftsmall](http://corpus-texmex.irisa.fr/) (10,000 base vectors of
dimension 128, 100 queries, and the ground-truth top-100 neighbours for each query). Fetch it once:

```
./scripts/fetch_siftsmall.ps1   # Windows PowerShell
./scripts/fetch_siftsmall.sh    # bash
```

It downloads and extracts into `data/siftsmall/` and checks the file sizes. `data/` is gitignored,
so every clone fetches its own copy. If FTP is blocked on your network, download `siftsmall.tar.gz`
manually from http://corpus-texmex.irisa.fr/ and extract it into `data/`.

## Phase 0

Design doc: [`doc/phase0.md`](doc/phase0.md).

| Component | Where |
|---|---|
| `Metadata`, `VectorRecord<T>` | `include/vecdb/vector_record.hpp` |
| `VectorStoreIO::read_vecs<T>` (`.fvecs` / `.ivecs` loader) | `include/vecdb/vector_store_io.hpp` |
| `squared_l2` | `include/vecdb/distance.hpp` |
| `flat_search` (exact brute-force kNN) | `include/vecdb/flat_search.hpp`, `src/flat_search.cpp` |
| `recall_at_k` | `include/vecdb/recall.hpp`, `src/recall.cpp` |

`vector-db-main` loads the dataset, runs `flat_search` with k=100 over all 100 queries, and reports
recall against the ground truth. It exits non-zero if the baseline is not reproduced:

```
$ ./build/vector-db-main
Loading siftsmall from 'data/siftsmall' ...
  base:        10000 x 128
  query:       100 x 128
  groundtruth: 100 x 100

Running flat_search with k=100 over 100 queries ...

  mean Recall@100: 1.000000
  min  Recall@100: 1.000000
  total time:     1021.44 ms
  per query:      10.21 ms

OK: Recall@100 = 1.0 baseline reproduced.
```

The dataset directory is resolved in this order: the first command-line argument if given, then
`data/siftsmall` relative to the working directory, then the absolute in-tree path baked in by
CMake. So the binary finds the data whether it is launched from the repo root or from
`build/Debug/`, and `vector-db-main /some/other/dir` still overrides everything.

`vecdb_tests` covers the loader, the search, and the recall metric with synthetic data (no dataset
required), plus the same Recall@100 = 1.0 baseline as a test — that one is skipped when `data/` has
not been fetched.

## Reading Material

- CMake, [how to setup a library-application architecture](https://cmake.org/cmake/help/latest/guide/tutorial/Getting%20Started%20with%20CMake.html)
- GoogleTest [setup](https://google.github.io/googletest/quickstart-cmake.html), [samples](https://google.github.io/googletest/samples.html)
- Commit messages, [conventional commits](https://www.conventionalcommits.org/en/v1.0.0/)
