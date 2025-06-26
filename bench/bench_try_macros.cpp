#include <benchmark/benchmark.h>
#include <result.hpp>
#include <string>

struct Error {
  std::string message;
};

template <typename T> using Result = cpp_result::Result<T, Error>;

template <typename T> inline Result<T> Ok(T value) {
  return Result<T>::Ok(std::forward<T>(value));
}
template <typename T> inline Result<T> Err(Error err) {
  return Result<T>::Err(std::move(err));
}

Result<double> divide_result(double a, double b) {
  if (b == 0.0)
    return Err<double>({"division by zero"});
  return Ok<double>(a / b);
}

// Benchmark for TRY macro
static void BM_TRY(benchmark::State &state) {
  int N = state.range(0);
  int ERR_EVERY = state.range(1);
  for (auto _ : state) {
    int errors = 0;
    double sum = 0;
    for (int i = 1; i <= N; ++i) {
      auto fn = [&]() -> Result<double> {
        double v = TRY(divide_result(i, (i % ERR_EVERY == 0) ? 0.0 : 2.0));
        return Ok<double>(v);
      };
      auto res = fn();
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
BENCHMARK(BM_TRY)->ArgsProduct({{10000, 100000, 1000000}, {100, 1000, 10000}});

// Benchmark for TRYL macro
static void BM_TRYL(benchmark::State &state) {
  int N = state.range(0);
  int ERR_EVERY = state.range(1);
  for (auto _ : state) {
    int errors = 0;
    double sum = 0;
    for (int i = 1; i <= N; ++i) {
      auto fn = [&]() -> Result<double> {
        TRYL(val, divide_result(i, (i % ERR_EVERY == 0) ? 0.0 : 2.0));
        return Ok<double>(val);
      };
      auto res = fn();
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
BENCHMARK(BM_TRYL)->ArgsProduct({{10000, 100000, 1000000}, {100, 1000, 10000}});

BENCHMARK_MAIN();
