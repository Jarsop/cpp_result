#include "../include/result.hpp"
#include <iostream>
#include <string>

struct Error {
  std::string message;
};

template <typename T> using MyResult = cpp_result::Result<T, Error>;
using VoidResult = cpp_result::Result<void, Error>;

MyResult<int> divide(int a, int b) noexcept {
  if (b == 0)
    return MyResult<int>::Err({"Division by zero"});
  return MyResult<int>::Ok(a / b);
}

int main() {
  auto result = divide(10, 2);
  if (result.is_ok()) {
    std::cout << "Result: " << result.unwrap() << "\n";
    std::cout << "Result (expect): " << result.expect("Should not fail")
              << "\n";
  } else {
    std::cout << "Error: " << result.unwrap_err().message << "\n";
    std::cout << "Error (expect_err): "
              << result.expect_err("Should not fail").message << "\n";
  }

  // Example: inspect and inspect_err
  result
      .inspect([](const int &v) {
        std::cout << "[inspect] Ok value: " << v << "\n";
      })
      .inspect_err([](const Error &e) {
        std::cout << "[inspect_err] Error: " << e.message << "\n";
      });

  auto mapped = result.map([](int val) { return val * 2; });
  if (mapped.is_ok()) {
    std::cout << "Mapped Result: " << mapped.unwrap() << "\n";
    std::cout << "Mapped Result (expect): " << mapped.expect("Should not fail")
              << "\n";
  } else {
    std::cout << "Mapped Error: " << mapped.unwrap_err().message << "\n";
    std::cout << "Mapped Error (expect_err): "
              << mapped.expect_err("Should not fail").message << "\n";
  }

  mapped.inspect(
      [](const int &v) { std::cout << "[inspect] mapped Ok: " << v << "\n"; });

  auto chained = result.and_then(
      [](int val) { return MyResult<std::string>::Ok(std::to_string(val)); });
  if (chained.is_ok()) {
    std::cout << "Chained Result: " << chained.unwrap() << "\n";
    std::cout << "Chained Result (expect): "
              << chained.expect("Should not fail") << "\n";
  } else {
    std::cout << "Chained Error: " << chained.unwrap_err().message << "\n";
    std::cout << "Chained Error (expect_err): "
              << chained.expect_err("Should not fail").message << "\n";
  }

  auto error_result = divide(10, 0);
  int val = error_result.unwrap_or(42);
  std::cout << "Fallback result: " << val << "\n";

  auto unwrapped = error_result.unwrap_or_else([]() {
    return -1; // Custom fallback logic
  });
  std::cout << "Unwrapped with fallback: " << unwrapped << "\n";

  auto mapped_err = error_result.map_err(
      [](const Error &err) { return Error{"Mapped Error: " + err.message}; });
  if (mapped_err.is_err())
    std::cout << "Mapped Error: " << mapped_err.unwrap_err().message << "\n";
  else
    std::cout << "Mapped Result: " << mapped_err.unwrap() << "\n";

  error_result.inspect_err([](const Error &e) {
    std::cout << "[inspect_err] mapped error: " << e.message << "\n";
  });

  VoidResult void_ok = VoidResult::Ok();
  if (void_ok.is_ok()) {
    std::cout << "Void Ok: success!\n";
    void_ok.expect("Void should not fail");
  } else {
    std::cout << "Void Ok: error!\n";
  }

  VoidResult void_err = VoidResult::Err({"Some void error"});
  if (void_err.is_err())
    std::cout << "Void Err: " << void_err.unwrap_err().message << "\n";

  auto void_map = void_ok.map([]() { return 1234; });
  if (void_map.is_ok())
    std::cout << "Void map Ok: " << void_map.unwrap() << "\n";

  auto void_map_err = void_err.map_err(
      [](const Error &e) { return Error{"Remapped: " + e.message}; });
  if (void_map_err.is_err())
    std::cout << "Void map_err: " << void_map_err.unwrap_err().message << "\n";

  auto void_and_then = void_ok.and_then(
      []() { return MyResult<std::string>::Ok("side effect ok"); });
  if (void_and_then.is_ok())
    std::cout << "Void and_then Ok: " << void_and_then.unwrap() << "\n";
}
