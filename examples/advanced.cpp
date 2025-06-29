#include <iostream>
#include <result.hpp>
#include <string>

using Result = cpp_result::Result<int, std::string>;
inline Result Ok(int val) { return Result::Ok(val); }
inline Result Err(std::string err) { return Result::Err(std::move(err)); }

// Parse an int from a string, with error reporting
Result parse_int(const std::string &s) {
  try {
    return Ok(std::stoi(s));
  } catch (...) {
    return Err("Invalid integer: " + s);
  }
}

// Divide two integers, error if division by zero
Result safe_div(int a, int b) {
  if (b == 0)
    return Err("Division by zero");
  return Ok(a / b);
}

// Compose: parse two strings, divide, and double the result if > 10
Result parse_div_and_double(const std::string &a, const std::string &b) {
  int x = TRY(parse_int(a));
  int y = TRY(parse_int(b));
  return safe_div(x, y)
      .and_then([](int v) {
        if (v > 10)
          return Ok(v * 2);
        return Err("Value too small: " + std::to_string(v));
      })
      .map([y](int v) { return v + y; });
}

int main() {
  for (auto &&[a, b] : {std::pair{"40", "2"}, std::pair{"18", "2"},
                        std::pair{"abc", "2"}, std::pair{"10", "0"}}) {
    auto res = parse_div_and_double(a, b);
    res.inspect([&](int v) {
         std::cout << "Result for (" << a << ", " << b << "): " << v << "\n";
       })
        .inspect_err([&](const std::string &e) {
          std::cout << "Error for (" << a << ", " << b << "): " << e << "\n";
        });
  }
  return 0;
}
