cmake_minimum_required(VERSION 3.12)
project(OrderBook LANGUAGES CXX)

# Set C++ standard (e.g., C++17 or C++20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Print build type for clarity
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Optimization flags for the two build types we actually use
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra -Wpedantic -fsanitize=address -fsanitize=undefined")

# Link flags for debug builds (for sanitizers)
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fsanitize=address -fsanitize=undefined")

# Performance-specific flags for Release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Enabling Link Time Optimization for Release build")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)  # Modern way to enable LTO
endif()


# Explicitly list all source files
set(ORDERBOOK_SOURCES
    src/OrderBook.cpp
    src/Order.cpp
)

# Find all headers
file(GLOB HEADERS "include/*.h" "include/*.hpp")

# Add your library
add_library(OrderBook ${ORDERBOOK_SOURCES} ${HEADERS})

# Make the 'include' folder visible to users of this library
target_include_directories(OrderBook PUBLIC include)

# --- Add your executable ---
add_executable(main_app src/main.cpp)
target_link_libraries(main_app PRIVATE OrderBook)

# --- Add benchmark executable ---
add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark PRIVATE OrderBook)

# --- Testing setup ---
enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# Add test executable
add_executable(
    unit_tests
    tests/test.cpp
)

target_link_libraries(
    unit_tests PRIVATE 
    OrderBook 
    GTest::gtest_main
)

target_compile_definitions(unit_tests PRIVATE 
    SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

gtest_discover_tests(unit_tests)