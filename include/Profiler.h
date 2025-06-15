#pragma once

#include <chrono>
#include <format>
#include <iostream>

class Profiler {
private:
  std::chrono::high_resolution_clock::time_point start_;
  const char *name_;

public:
  explicit Profiler(const char *name = "Operation")
      : start_(std::chrono::high_resolution_clock::now()), name_(name) {}

  ~Profiler() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_);

    printf("%s took: %ld ns\n", name_, duration.count());
  }
};