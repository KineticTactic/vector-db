cmake_minimum_required(VERSION 3.23)
project(vector-db)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ----------------VECDB-------------------
add_library(vecdb)
target_sources(vecdb
  PRIVATE
    src/distance.cpp
    src/flat_search.cpp
    src/vector_store_io.cpp

  PUBLIC
    FILE_SET vecdbHeaders
    TYPE HEADERS
    BASE_DIRS 
      include
    FILES 
      include/vecdb/vector_record.hpp
      include/vecdb/vector_store_io.hpp
      include/vecdb/distance.hpp
      include/vecdb/flat_search.hpp
)

# ------------------EXAMPLES-----------------------
add_executable(flat_search_recall
  examples/flat_search_recall.cpp
)
target_link_libraries(flat_search_recall
  PRIVATE
    vecdb
)
target_compile_definitions(flat_search_recall PRIVATE
    DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
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
  vecdb_tests
  test/vector_record_test.cpp
  test/vector_store_io_test.cpp
  test/distance_test.cpp
  test/flat_search_test.cpp
  test/siftsmall_test.cpp
)
target_compile_definitions(vecdb_tests PRIVATE
    DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)
target_link_libraries(
  vecdb_tests
  vecdb
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(vecdb_tests)

# ---------------------- SANITIZER ------------------------
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(ENABLE_SANITIZERS)
    add_library(sanitizers INTERFACE)

    target_compile_options(sanitizers INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )

    target_link_options(sanitizers INTERFACE
        -fsanitize=address,undefined
    )

    target_link_libraries(vecdb PUBLIC sanitizers)
endif()
