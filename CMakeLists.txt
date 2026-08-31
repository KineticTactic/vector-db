cmake_minimum_required(VERSION 3.23)
project(vector-db)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(vecdb)
target_sources(vecdb
  PRIVATE
    src/flat_search.cpp
    src/recall.cpp

  PUBLIC
    FILE_SET vecdbHeaders
    TYPE HEADERS
    BASE_DIRS 
      include
    FILES 
      include/vecdb/distance.hpp
      include/vecdb/flat_search.hpp
      include/vecdb/recall.hpp
      include/vecdb/vector_record.hpp
      include/vecdb/vector_store_io.hpp
)

add_executable(vector-db-main)
target_sources(vector-db-main
  PRIVATE
    main.cpp
)

target_link_libraries(vector-db-main
  PRIVATE
    vecdb
)

# Fallback dataset location, used when the working directory is not the repo
# root (multi-config generators run the binary from build/<config>/).
target_compile_definitions(vector-db-main
  PRIVATE
    VECDB_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/siftsmall"
)


# --------------- GOOGLE TEST -------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  hello_test
  test/hello_test.cpp
)
target_link_libraries(
  hello_test
  GTest::gtest_main
)

add_executable(
  vecdb_tests
  test/flat_search_test.cpp
  test/recall_test.cpp
  test/siftsmall_recall_test.cpp
  test/vector_store_io_test.cpp
)
target_link_libraries(
  vecdb_tests
  vecdb
  GTest::gtest_main
)
# Baked in so the dataset is found no matter where the test binary is run from.
target_compile_definitions(vecdb_tests
  PRIVATE
    VECDB_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/siftsmall"
)

include(GoogleTest)
gtest_discover_tests(hello_test)
gtest_discover_tests(vecdb_tests)
