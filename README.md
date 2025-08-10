# High-Performance OrderBook Implementation

A thread-safe, multi-order type OrderBook implementation in C++ designed for low-latency trading systems.

## Features

- **Multiple Order Types**: Good Till Cancel (GTC), Fill And Kill (FAK), Fill Or Kill (FOK), Market, Good For Day (GFD)
- **Thread-Safe Operations**: Concurrent order processing with proper synchronization
- **Price-Time Priority**: FIFO matching within price levels
- **Automatic Order Management**: Background thread for Good For Day order pruning
- **Comprehensive Testing**: Full test suite with Google Test framework

## Quick Start

### Prerequisites

- CMake 3.28+
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- Git (for dependency management)

### Build Instructions

**Option 1: Quick Build Scripts (Recommended)**

```bash
# Clone the repository
git clone https://github.com/daronoh/OrderBook.git
cd OrderBook

# For development and debugging
./build-debug.sh

# For performance testing and benchmarks
./build-release.sh
```

**Option 2: Manual CMake Build**

```bash
# Create build directory
mkdir -p build
cd build

# Configure (choose one)
cmake -DCMAKE_BUILD_TYPE=Debug ..     # For debugging
cmake -DCMAKE_BUILD_TYPE=Release ..   # For performance

# Build
make -j$(nproc)
```

### Running the Application

**Debug Build:**
```bash
./build-debug/unit_tests     # Run tests
./build-debug/main_app       # Demo application  
./build-debug/benchmark      # Performance benchmark (slower)
```

**Release Build:**
```bash
./build-release/unit_tests   # Run tests
./build-release/main_app     # Demo application
./build-release/benchmark    # Performance benchmark (optimized)
```

### Quick Example

```cpp
#include "OrderBook.h"

int main() {
    Orderbook orderbook;
    
    // Add a buy order
    auto buyOrder = std::make_shared<Order>(
        OrderType::GoodTillCancel, 1, Side::Buy, 100, 10);
    orderbook.AddOrder(buyOrder);
    
    // Add a matching sell order
    auto sellOrder = std::make_shared<Order>(
        OrderType::GoodTillCancel, 2, Side::Sell, 100, 5);
    auto trades = orderbook.AddOrder(sellOrder);
    
    // Check if trade occurred
    std::cout << "Trades executed: " << trades.size() << std::endl;
    return 0;
}
```

## Architecture

### Core Components

- **OrderBook**: Main engine managing orders and matching
- **Order**: Individual order representation with various types
- **Trade**: Result of successful order matching
- **PriceLevelInfo**: Market data aggregation by price level

### Data Structures

- **Bids**: `std::map<Price, OrderPointers, std::greater<Price>>` (descending)
- **Asks**: `std::map<Price, OrderPointers, std::less<Price>>` (ascending)
- **Order Registry**: `std::unordered_map<OrderId, OrderEntry>` for O(1) lookup

## Performance Characteristics

### Build Configurations

| Build Type | Optimization | Debug Info | Use Case | Expected Performance |
|------------|-------------|------------|----------|---------------------|
| **Debug** | `-O0` | Full (`-g`) | Development, debugging | ~10x slower |
| **Release** | `-O3 -march=native` | None | Benchmarking, production | Maximum speed |

### Algorithm Complexity

- **Add Order**: O(log n) time complexity
- **Cancel Order**: O(1) time complexity  
- **Modify Order**: O(log n) time complexity
- **Memory Usage**: O(n) where n = number of active orders

### Performance Expectations (Release Build)

**Actual Benchmark Results:**
- **Average Latency**: 211-326 nanoseconds per operation
- **Peak Throughput**: 4.35M operations per second
- **Minimum Latency**: 38 nanoseconds (best case)
- **Memory**: ~100 bytes per active order

**Benchmark Summary:**

| Test | Avg Latency | Throughput |
|------|-------------|------------|
| Mixed Operations | 211 ns | 4.35M ops/sec |
| Add Orders | 270-326 ns | 2.69-3.2M ops/sec |

*Note: Results from Release build (-O3 -march=native) on modern x86_64 CPU*

## Testing

The project includes comprehensive test coverage:

```bash
# Run all tests
./build/unit_tests

# Test scenarios include:
# - Basic order matching
# - Market order execution  
# - Fill And Kill behavior
# - Fill Or Kill scenarios
# - Order cancellation
# - Order modification
```

## Benchmarks

Run performance benchmarks to measure OrderBook latency and throughput:

```bash
# Build optimized version first
./build-release.sh

# Run benchmark suite
./build-release/benchmark
```

**Latest Results (Release Build):**
```
=== Add 10,000 Orders ===
Average Latency: 326 ns
Min Latency: 106 ns  
Max Latency: 24,284 ns
Throughput: 2.69M ops/sec

=== Mixed Operations (5,000) ===  
Average Latency: 211 ns
Min Latency: 38 ns
Max Latency: 15,802 ns
Throughput: 4.35M ops/sec
```

## Order Types

| Type | Description | Behavior |
|------|-------------|----------|
| **GoodTillCancel** | Remains active until cancelled | Standard limit order |
| **FillAndKill** | Fill immediately, cancel remainder | Immediate execution |
| **FillOrKill** | Fill completely or cancel entirely | All-or-nothing |
| **Market** | Execute at best available price | Converted to FAK |
| **GoodForDay** | Auto-cancel at market close | Time-based expiry |

## Future Optimizations

1. **Lock-free data structures** - Eliminate mutex contention
2. **Memory pool allocation** - Reduce allocation overhead
3. **Multi-threaded pre-processing** - Parallel order validation
4. **SPSC/MPSC Queues** - Efficient order pipeline
5. **CPU Cache Optimization** - Improve memory access patterns
6. **Branch Prediction Optimization** - Reduce conditional overhead
7. **SIMD Instructions** - Vectorized operations
8. **Template Metaprogramming** - Compile-time optimizations

## Documentation

- [Technical Report](TECHNICAL_REPORT.md) - Comprehensive analysis and performance metrics

