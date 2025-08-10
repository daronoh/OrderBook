#!/bin/bash

echo "======================================"
echo "Building OrderBook in RELEASE mode"
echo "======================================"

# Create build directory if it doesn't exist
mkdir -p build-release
cd build-release

# Configure with Release build type
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build the project
make -j$(nproc)

echo ""
echo "Release build completed!"
echo "Executables available:"
echo "  - ./build-release/unit_tests     (run tests)"
echo "  - ./build-release/main_app       (demo application)"
echo "  - ./build-release/benchmark      (performance benchmark)"
echo ""
echo "Note: This is a RELEASE build with:"
echo "  - Full optimizations (-O3)"
echo "  - Native CPU optimizations (-march=native)"
echo "  - Link Time Optimization (LTO)"
echo "  - No debug symbols (smaller binaries)"
echo "  - Maximum performance for benchmarking"
echo ""
echo "🚀 Ready for performance testing!"
