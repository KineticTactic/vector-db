cmake_minimum_required(VERSION 3.23)
project(vector-db)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(vecdb)
target_sources(vecdb
  PRIVATE
    src/vector.cpp

  PUBLIC
    FILE_SET vecdbHeaders
    TYPE HEADERS
    BASE_DIRS 
      include
    FILES 
      include/vecdb/vector.hpp
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
  test/vector_record_test.cpp
  test/vector_store_io_test.cpp
)
target_link_libraries(
  vecdb_tests
  vecdb
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(hello_test)
gtest_discover_tests(vecdb_tests)

