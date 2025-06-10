cmake_minimum_required(VERSION 3.12)
project(OrderBook LANGUAGES CXX)

# Set C++ standard (e.g., C++17 or C++20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt REQUIRED)

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

target_link_libraries(OrderBook fmt::fmt)

# --- Add your executable ---
add_executable(main_app src/main.cpp)
target_link_libraries(main_app PRIVATE OrderBook)