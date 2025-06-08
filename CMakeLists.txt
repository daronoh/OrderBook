cmake_minimum_required(VERSION 3.12)
project(OrderBook LANGUAGES CXX)

# Set C++ standard (e.g., C++17 or C++20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Add your library ---
# Create a library from OrderBook.cpp
add_library(OrderBook src/OrderBook.cpp include/OrderBook.hpp)

# Make the 'include' folder visible to users of this library
target_include_directories(OrderBook PUBLIC include)

# --- Add your executable ---
add_executable(main_app src/main.cpp)

# Link the OrderBook library to your executable
target_link_libraries(main_app PRIVATE OrderBook)