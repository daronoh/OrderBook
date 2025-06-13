cmake_minimum_required(VERSION 3.12)
project(OrderBook LANGUAGES CXX)

# Set C++ standard (e.g., C++17 or C++20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


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