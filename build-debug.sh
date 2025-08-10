#!/bin/bash

echo "==================================="
echo "Building OrderBook in DEBUG mode"
echo "==================================="

# Create build directory if it doesn't exist
mkdir -p build-debug
cd build-debug

# Configure with Debug build type
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build the project
make -j$(nproc)

echo ""
echo "Debug build completed!"
echo "Executables available:"
echo "  - ./build-debug/unit_tests     (run tests)"
echo "  - ./build-debug/main_app       (demo application)"
echo "  - ./build-debug/benchmark      (performance benchmark)"
echo ""
echo "Note: This is a DEBUG build with:"
echo "  - No optimizations (-O0)"
echo "  - Debug symbols (-g)"
echo "  - All warnings enabled"
echo "  - Slower performance for accurate debugging"
