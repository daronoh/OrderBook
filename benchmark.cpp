#include <chrono>
#include <iostream>
#include <random>
#include <vector>

#include "OrderBook.h"

class PerformanceBenchmark {
public:
  struct BenchmarkResult {
    double avgLatencyNs;
    double maxLatencyNs;
    double minLatencyNs;
    double throughputOpsPerSec;
    size_t totalOperations;
  };

  static BenchmarkResult BenchmarkAddOrders(int numOrders) {
    Orderbook orderbook;
    std::vector<double> latencies;
    latencies.reserve(numOrders);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> priceDist(90, 110);
    std::uniform_int_distribution<> quantityDist(1, 100);
    std::uniform_int_distribution<> sideDist(0, 1);

    auto startTotal = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numOrders; ++i) {
      auto order =
          std::make_shared<Order>(OrderType::GoodTillCancel, i + 1,
                                  sideDist(gen) ? Side::Buy : Side::Sell,
                                  priceDist(gen), quantityDist(gen));

      auto start = std::chrono::high_resolution_clock::now();
      orderbook.AddOrder(order);
      auto end = std::chrono::high_resolution_clock::now();

      auto latency =
          std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
              .count();
      latencies.push_back(latency);
    }

    auto endTotal = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
                         endTotal - startTotal)
                         .count();

    // Calculate statistics
    double sum = 0;
    double maxLat = latencies[0];
    double minLat = latencies[0];

    for (double lat : latencies) {
      sum += lat;
      if (lat > maxLat)
        maxLat = lat;
      if (lat < minLat)
        minLat = lat;
    }

    double avgLatency = sum / latencies.size();
    double throughput = (static_cast<double>(numOrders) * 1e9) / totalTime;

    return {avgLatency, maxLat, minLat, throughput,
            static_cast<size_t>(numOrders)};
  }

  // Benchmark different operation types
  static BenchmarkResult BenchmarkMixedOperations(int numOperations) {
    Orderbook orderbook;
    std::vector<double> latencies;
    latencies.reserve(numOperations);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> priceDist(95, 105);
    std::uniform_int_distribution<> quantityDist(1, 100);
    std::uniform_int_distribution<> sideDist(0, 1);
    std::uniform_int_distribution<> operationDist(
        0, 2); // 0=Add, 1=Cancel, 2=Modify

    // Pre-populate some orders
    std::vector<OrderId> activeOrders;
    for (int i = 0; i < 50; ++i) {
      auto order =
          std::make_shared<Order>(OrderType::GoodTillCancel, i + 1,
                                  sideDist(gen) ? Side::Buy : Side::Sell,
                                  priceDist(gen), quantityDist(gen));
      orderbook.AddOrder(order);
      activeOrders.push_back(i + 1);
    }

    auto startTotal = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numOperations; ++i) {
      auto start = std::chrono::high_resolution_clock::now();

      int operation = operationDist(gen);
      if (operation == 0 || activeOrders.empty()) {
        // Add order
        auto order =
            std::make_shared<Order>(OrderType::GoodTillCancel, i + 1000,
                                    sideDist(gen) ? Side::Buy : Side::Sell,
                                    priceDist(gen), quantityDist(gen));
        orderbook.AddOrder(order);
        activeOrders.push_back(i + 1000);
      } else if (operation == 1 && !activeOrders.empty()) {
        // Cancel order
        std::uniform_int_distribution<> orderIndexDist(0,
                                                       activeOrders.size() - 1);
        int index = orderIndexDist(gen);
        OrderId orderId = activeOrders[index];
        orderbook.CancelOrder(orderId);
        activeOrders.erase(activeOrders.begin() + index);
      } else if (!activeOrders.empty()) {
        // Modify order (cancel + add)
        std::uniform_int_distribution<> orderIndexDist(0,
                                                       activeOrders.size() - 1);
        int index = orderIndexDist(gen);
        OrderId oldOrderId = activeOrders[index];

        orderbook.CancelOrder(oldOrderId);
        activeOrders.erase(activeOrders.begin() + index);

        auto newOrder =
            std::make_shared<Order>(OrderType::GoodTillCancel, i + 2000,
                                    sideDist(gen) ? Side::Buy : Side::Sell,
                                    priceDist(gen), quantityDist(gen));
        orderbook.AddOrder(newOrder);
        activeOrders.push_back(i + 2000);
      }

      auto end = std::chrono::high_resolution_clock::now();
      auto latency =
          std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
              .count();
      latencies.push_back(latency);
    }

    auto endTotal = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
                         endTotal - startTotal)
                         .count();

    // Calculate statistics
    double sum = 0;
    double maxLat = latencies[0];
    double minLat = latencies[0];

    for (double lat : latencies) {
      sum += lat;
      if (lat > maxLat)
        maxLat = lat;
      if (lat < minLat)
        minLat = lat;
    }

    double avgLatency = sum / latencies.size();
    double throughput = (static_cast<double>(numOperations) * 1e9) / totalTime;

    return {avgLatency, maxLat, minLat, throughput,
            static_cast<size_t>(numOperations)};
  }

  static void PrintPercentiles(const std::vector<double> &latencies) {
    auto sortedLatencies = latencies;
    std::sort(sortedLatencies.begin(), sortedLatencies.end());

    size_t size = sortedLatencies.size();
    std::cout << "50th percentile: " << sortedLatencies[size * 50 / 100]
              << " ns" << std::endl;
    std::cout << "90th percentile: " << sortedLatencies[size * 90 / 100]
              << " ns" << std::endl;
    std::cout << "95th percentile: " << sortedLatencies[size * 95 / 100]
              << " ns" << std::endl;
    std::cout << "99th percentile: " << sortedLatencies[size * 99 / 100]
              << " ns" << std::endl;
    std::cout << "99.9th percentile: " << sortedLatencies[size * 999 / 1000]
              << " ns" << std::endl;
  }

  static void PrintResults(const BenchmarkResult &result,
                           const std::string &testName) {
    std::cout << "\n=== " << testName << " ===" << std::endl;
    std::cout << "Total Operations: " << result.totalOperations << std::endl;
    std::cout << "Average Latency: " << result.avgLatencyNs << " ns"
              << std::endl;
    std::cout << "Min Latency: " << result.minLatencyNs << " ns" << std::endl;
    std::cout << "Max Latency: " << result.maxLatencyNs << " ns" << std::endl;
    std::cout << "Throughput: " << result.throughputOpsPerSec << " ops/sec"
              << std::endl;
  }
};

int main() {
  std::cout << "OrderBook Performance Benchmark" << std::endl;
  std::cout << "==============================" << std::endl;

  // Benchmark different order counts
  std::vector<int> orderCounts = {1000, 5000, 10000};

  for (int count : orderCounts) {
    auto result = PerformanceBenchmark::BenchmarkAddOrders(count);
    PerformanceBenchmark::PrintResults(result, "Add " + std::to_string(count) +
                                                   " Orders");
  }

  std::cout << "\n\n=== Mixed Operations Benchmark ===" << std::endl;
  auto mixedResult = PerformanceBenchmark::BenchmarkMixedOperations(5000);
  PerformanceBenchmark::PrintResults(mixedResult, "Mixed Operations (5000)");

  return 0;
}
