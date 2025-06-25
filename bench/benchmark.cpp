#include <benchmark/benchmark.h>
#include <chrono>
#include <iostream>
#include <result.hpp>
#include <stdexcept>
#include <string>
#include <vector>

struct Error {
  std::string message;
};

template <typename T> using MyResult = cpp_result::Result<T, Error>;

double divide_exc(double a, double b) {
  if (b == 0.0)
    throw std::runtime_error("division by zero");
  return a / b;
}

bool divide_code(double a, double b, double &out) {
  if (b == 0.0)
    return false;
  out = a / b;
  return true;
}

MyResult<double> divide_result(double a, double b) {
  if (b == 0.0)
    return MyResult<double>::Err({"division by zero"});
  return MyResult<double>::Ok(a / b);
}

class DivideFixture : public benchmark::Fixture {};

BENCHMARK_DEFINE_F(DivideFixture, Exceptions)(benchmark::State &state) {
  int N = state.range(0);
  int ERR_EVERY = state.range(1);
  for (auto _ : state) {
    int errors = 0;
    double sum = 0;
    for (int i = 1; i <= N; ++i) {
      try {
        sum += divide_exc(i, (i % ERR_EVERY == 0) ? 0.0 : 2.0);
      } catch (const std::exception &) {
        ++errors;
      }
    }
    benchmark::DoNotOptimize(sum);
    benchmark::DoNotOptimize(errors);
    state.counters["errors"] = errors;
  }
}
BENCHMARK_REGISTER_F(DivideFixture, Exceptions)
    ->ArgsProduct({{10000, 100000, 1000000}, {100, 1000, 10000}});

BENCHMARK_DEFINE_F(DivideFixture, ErrorCode)(benchmark::State &state) {
  int N = state.range(0);
  int ERR_EVERY = state.range(1);
  for (auto _ : state) {
    int errors = 0;
    double sum = 0;
    for (int i = 1; i <= N; ++i) {
      double out = 0;
      if (divide_code(i, (i % ERR_EVERY == 0) ? 0.0 : 2.0, out)) {
        sum += out;
      } else {
        ++errors;
      }
    }
    benchmark::DoNotOptimize(sum);
    benchmark::DoNotOptimize(errors);
    state.counters["errors"] = errors;
  }
}
BENCHMARK_REGISTER_F(DivideFixture, ErrorCode)
    ->ArgsProduct({{10000, 100000, 1000000}, {100, 1000, 10000}});

BENCHMARK_DEFINE_F(DivideFixture, Result)(benchmark::State &state) {
  int N = state.range(0);
  int ERR_EVERY = state.range(1);
  for (auto _ : state) {
    int errors = 0;
    double sum = 0;
    for (int i = 1; i <= N; ++i) {
      auto res = divide_result(i, (i % ERR_EVERY == 0) ? 0.0 : 2.0);
      if (res.is_ok()) {
        sum += res.unwrap();
      } else {
        ++errors;
      }
    }
    benchmark::DoNotOptimize(sum);
    benchmark::DoNotOptimize(errors);
    state.counters["errors"] = errors;
  }
}
BENCHMARK_REGISTER_F(DivideFixture, Result)
    ->ArgsProduct({{10000, 100000, 1000000}, {100, 1000, 10000}});

BENCHMARK_MAIN();
