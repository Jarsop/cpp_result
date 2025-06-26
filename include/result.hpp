/**
 * @mainpage cpp_result
 * @brief Modern Rust-like Result<T, E> for C++17.
 *
 * cpp_result is a header-only, ergonomic Result type for C++17 inspired by
 * Rust, with combinators, helpers, and void specialization.
 *
 * - See the API reference for details.
 * - Quickstart:
 *   @code
 *   #include <result.hpp>
 *   struct Error { std::string message; };
 *   using Result = cpp_result::Result<int, Error>;
 *   Result divide(int a, int b) {
 *     if (b == 0) return cpp_result::Err<int, Error>({"Division by zero"});
 *     return cpp_result::Ok<int, Error>(a / b);
 *   }
 *   int main() {
 *     auto r = divide(10, 2);
 *     if (r.is_ok()) std::cout << r.unwrap() << '\n';
 *     else std::cout << r.unwrap_err().message << '\n';
 *   }
 *   @endcode
 */

// result.hpp - Rust-like Result<T, E> for C++17
// SPDX-License-Identifier: MIT
//
// This header provides a modern, ergonomic Result type for C++17 inspired by
// Rust. It supports combinators, helpers, and void specialization.
//
// All API is in the cpp_result namespace.
//
// --- API OVERVIEW ---
//
// Construction:
//   Result<T, E>::Ok(T)
//   Result<T, E>::Err(E)
//   Result<void, E>::Ok()
//   Result<void, E>::Err(E)
//   cpp_result::Ok<T, E>(T)
//   cpp_result::Err<T, E>(E)
//   cpp_result::Ok<E>()
//   cpp_result::Err<E>(E)
//
// Query:
//   is_ok(), is_err()
//
// Unwrap:
//   unwrap(), unwrap_err(), unwrap_or(value), unwrap_or_else(fn)
//   expect(msg), expect_err(msg)
//
// Combinators:
//   map(fn), map_err(fn), and_then(fn)
//   inspect(fn), inspect_err(fn)
//
// See the documentation below each function for usage examples.

#pragma once

#include <cstdio>
#include <cstdlib>
#include <type_traits>
#include <utility>

#define EXPECT_OR_ABORT(cond, msg)                                             \
  do {                                                                         \
    if (!(cond)) {                                                             \
      std::fputs(msg, stderr);                                                 \
      std::fputc('\n', stderr);                                                \
      std::abort();                                                            \
    }                                                                          \
  } while (0)

namespace cpp_result {

/**
 * @brief Result<T, E> - Holds either a value (Ok) or an error (Err).
 *
 * @tparam T Value type
 * @tparam E Error type
 *
 * Example:
 * @code
 * auto r = cpp_result::Ok<int, std::string>(42);
 * if (r.is_ok()) std::cout << r.unwrap();
 * @endcode
 */
template <typename T, typename E> class [[nodiscard]] Result {
  static_assert(!std::is_reference_v<T> && !std::is_reference_v<E>,
                "Result<T,E> does not support reference types");

public:
  /**
   * @brief Construct an Ok result.
   * @param val Value to store
   * @return Ok result
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Ok(42);
   * // or
   * auto r2 = cpp_result::Ok<int, std::string>(42);
   * @endcode
   */
  static inline Result Ok(T val) noexcept { return Result(std::move(val)); }

  /**
   * @brief Construct an Err result.
   * @param err Error to store
   * @return Err result
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Err("fail");
   * // or
   * auto r2 = cpp_result::Err<int, std::string>("fail");
   * @endcode
   */
  static inline Result Err(E err) noexcept { return Result(std::move(err)); }

  /**
   * @brief Returns true if the result is Ok.
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Ok(1);
   * if (r.is_ok()) {  ...  }
   * @endcode
   */
  constexpr bool is_ok() const noexcept { return is_ok_; }

  /**
   * @brief Returns true if the result is Err.
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Err("fail");
   * if (r.is_err()) {  ...  }
   * @endcode
   */
  constexpr bool is_err() const noexcept { return !is_ok_; }

  /**
   * @brief Unwraps the value. Aborts if Err.
   * @return T&
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Ok(42);
   * int v = r.unwrap(); // v == 42
   * @endcode
   */
  inline T &unwrap() noexcept {
    EXPECT_OR_ABORT(is_ok_, "unwrap called on Result::Err()");
    return data_.value;
  }
  inline const T &unwrap() const noexcept {
    EXPECT_OR_ABORT(is_ok_, "unwrap called on Result::Err()");
    return data_.value;
  }

  /**
   * @brief Unwraps the error. Aborts if Ok.
   * @return E&
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Err("fail");
   * std::string e = r.unwrap_err(); // e == "fail"
   * @endcode
   */
  inline E &unwrap_err() noexcept {
    EXPECT_OR_ABORT(!is_ok_, "unwrap_err called on Result::Ok()");
    return data_.error;
  }
  inline const E &unwrap_err() const noexcept {
    EXPECT_OR_ABORT(!is_ok_, "unwrap_err called on Result::Ok()");
    return data_.error;
  }

  /**
   * @brief Returns value if Ok, else returns default_value.
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Err("fail");
   * int v = r.unwrap_or(123); // v == 123
   * @endcode
   */
  inline T unwrap_or(T default_value) const noexcept {
    return is_ok_ ? data_.value : default_value;
  }

  /**
   * @brief Returns value if Ok, else calls func().
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Err("fail");
   * int v = r.unwrap_or_else([]{ return 123; }); // v == 123
   * @endcode
   */
  template <typename F>
  inline T unwrap_or_else(F &&func) const noexcept(noexcept(func())) {
    return is_ok_ ? data_.value : std::forward<F>(func)();
  }

  /**
   * @brief Maps the value if Ok, else propagates Err.
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Ok(21);
   * auto mapped = r.map([](int v){ return v*2; }); // mapped.unwrap() == 42
   * @endcode
   */
  template <typename F, typename U = std::invoke_result_t<F, T>>
  Result<U, E> map(F &&func) const noexcept(noexcept(func(std::declval<T>()))) {
    if (is_ok_)
      return Result<U, E>::Ok(func(data_.value));
    return Result<U, E>::Err(data_.error);
  }

  /**
   * @brief Maps the error if Err, else propagates Ok.
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Err("fail");
   * auto mapped = r.map_err([](const std::string& e){ return e+"!"; });
   * // mapped.unwrap_err() == "fail!"
   * @endcode
   */
  template <typename F, typename E2 = std::invoke_result_t<F, E>>
  Result<T, E2> map_err(F &&func) const
      noexcept(noexcept(func(std::declval<E>()))) {
    if (is_ok_)
      return Result<T, E2>::Ok(data_.value);
    return Result<T, E2>::Err(func(data_.error));
  }

  /**
   * @brief Chains another result-producing function if Ok, else propagates Err.
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Ok(1);
   * auto chained = r.and_then([](int v){
   *    return cpp_result::Ok<int, std::string>(v+1);
   * });
   * // chained.unwrap() == 2
   * @endcode
   */
  template <typename F, typename R = typename std::invoke_result_t<F, T>>
  R and_then(F &&func) const noexcept(noexcept(func(std::declval<T>()))) {
    if (is_ok_)
      return func(data_.value);
    return R::Err(data_.error);
  }

  /**
   * @brief Unwraps the value or aborts with a custom message if Err.
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Ok(42);
   * int v = r.expect("should not fail"); // v == 42
   * @endcode
   */
  T &expect(const char *msg) noexcept {
    EXPECT_OR_ABORT(is_ok_, msg);
    return data_.value;
  }
  const T &expect(const char *msg) const noexcept {
    EXPECT_OR_ABORT(is_ok_, msg);
    return data_.value;
  }

  /**
   * @brief Unwraps the error or aborts with a custom message if Ok.
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Err("fail");
   * std::string e = r.expect_err("should not fail"); // e == "fail"
   * @endcode
   */
  E &expect_err(const char *msg) noexcept {
    EXPECT_OR_ABORT(!is_ok_, msg);
    return data_.error;
  }
  const E &expect_err(const char *msg) const noexcept {
    EXPECT_OR_ABORT(!is_ok_, msg);
    return data_.error;
  }

  /**
   * @brief Calls func(value) if Ok, returns self.
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Ok(42);
   * r.inspect([](int v){ std::cout << v; });
   * @endcode
   */
  template <typename F>
  const Result &inspect(F &&func) const
      noexcept(noexcept(func(std::declval<const T &>()))) {
    if (is_ok_)
      func(data_.value);
    return *this;
  }

  /**
   * @brief Calls func(error) if Err, returns self.
   * @code
   * using MyResult = cpp_result::Result<int, std::string>;
   * auto r = MyResult::Err("fail");
   * r.inspect_err([](const std::string& e){ std::cout << e; });
   * @endcode
   */
  template <typename F>
  const Result &inspect_err(F &&func) const
      noexcept(noexcept(func(std::declval<const E &>()))) {
    if (!is_ok_)
      func(data_.error);
    return *this;
  }

  ~Result() { destroy(); }

  // Move semantics
  Result(Result &&other) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                  std::is_nothrow_move_constructible_v<E>)
      : is_ok_(other.is_ok_) {
    if (is_ok_)
      new (&data_.value) T(std::move(other.data_.value));
    else
      new (&data_.error) E(std::move(other.data_.error));
  }

  Result &
  operator=(Result &&other) noexcept(std::is_nothrow_move_assignable_v<T> &&
                                     std::is_nothrow_move_assignable_v<E>) {
    if (this != &other) {
      destroy();
      is_ok_ = other.is_ok_;
      if (is_ok_)
        new (&data_.value) T(std::move(other.data_.value));
      else
        new (&data_.error) E(std::move(other.data_.error));
    }
    return *this;
  }

  // Conditional copy semantics
  Result(const Result &other) noexcept(
      std::is_nothrow_copy_constructible_v<T> &&
      std::is_nothrow_copy_constructible_v<E>)
      : is_ok_(other.is_ok_) {
    if (is_ok_)
      new (&data_.value) T(other.data_.value);
    else
      new (&data_.error) E(other.data_.error);
  }

  Result &operator=(const Result &other) noexcept(
      std::is_nothrow_copy_assignable_v<T> &&
      std::is_nothrow_copy_assignable_v<E>) {
    if (this != &other) {
      destroy();
      is_ok_ = other.is_ok_;
      if (is_ok_)
        new (&data_.value) T(other.data_.value);
      else
        new (&data_.error) E(other.data_.error);
    }
    return *this;
  }

private:
  union Data {
    T value;
    E error;
    Data() {}
    ~Data() {}
  } data_;
  bool is_ok_;

  explicit Result(T val) noexcept : is_ok_(true) {
    new (&data_.value) T(std::move(val));
  }

  explicit Result(E err) noexcept : is_ok_(false) {
    new (&data_.error) E(std::move(err));
  }

  void destroy() noexcept { is_ok_ ? data_.value.~T() : data_.error.~E(); }
};

/**
 * @brief Result<void, E> - Specialization for operations that return no
 * value, only success or error.
 *
 * @tparam E Error type
 *
 * Example:
 * @code
 * auto r = cpp_result::Ok<std::string>();
 * if (r.is_ok()) std::cout << "All good!";
 * else std::cout << r.unwrap_err();
 * @endcode
 */
template <typename E> class [[nodiscard]] Result<void, E> {
  static_assert(!std::is_reference_v<E>,
                "Result<void,E> does not support reference types");

public:
  /**
   * @brief Construct an Ok result.
   * @return Ok result
   * @code
   * using MyResult = cpp_result::Result<void, std::string>;
   * auto r = MyResult::Ok();
   * // or
   * auto r2 = cpp_result::Ok<std::string>();
   * @endcode
   */
  static inline Result Ok() noexcept { return Result(); }

  /**
   * @brief Construct an Err result.
   * @param err Error to store
   * @return Err result
   * @code
   * using MyResult = cpp_result::Result<void, std::string>;
   * auto r = MyResult::Err("fail");
   * // or
   * auto r2 = cpp_result::Err<std::string>("fail");
   * @endcode
   */
  static inline Result Err(E err) noexcept { return Result(std::move(err)); }

  /**
   * @brief Returns true if the result is Ok.
   * @code
   * using MyResult = cpp_result::Result<void, std::string>;
   * auto r = MyResult::Ok();
   * if (r.is_ok()) {  ...  }
   * @endcode
   */
  inline bool is_ok() const noexcept { return is_ok_; }

  /**
   * @brief Returns true if the result is Err.
   * @code
   * using MyResult = cpp_result::Result<void, std::string>;
   * auto r = MyResult::Err("fail");
   * if (r.is_err()) {  ...  }
   * @endcode
   */
  inline bool is_err() const noexcept { return !is_ok_; }

  /**
   * @brief Unwraps the value. Aborts if Err.
   * @code
   * using MyResult = cpp_result::Result<void, std::string>;
   * auto r = MyResult::Ok();
   * r.unwrap(); // does nothing if Ok
   * @endcode
   */
  inline void unwrap() noexcept {
    EXPECT_OR_ABORT(is_ok_, "unwrap called on Result::Err()");
  }

  /**
   * @brief Unwraps the error. Aborts if Ok.
   * @return E&
   * @code
   * using MyResult = cpp_result::Result<void, std::string>;
   * auto r = MyResult::Err("fail");
   * std::string e = r.unwrap_err(); // e == "fail"
   * @endcode
   */
  inline E &unwrap_err() noexcept {
    EXPECT_OR_ABORT(!is_ok_, "unwrap_err called on Result::Ok()");
    return error_;
  }
  inline const E &unwrap_err() const noexcept {
    EXPECT_OR_ABORT(!is_ok_, "unwrap_err called on Result::Ok()");
    return error_;
  }

  /**
   * @brief Returns unit type if Ok, else returns error.
   * @code
   * using MyResult = cpp_result::Result<void, std::string>;
   * auto r = MyResult::Ok();
   * auto mapped = r.map([]{ return 42; }); // mapped.unwrap() == 42
   * @endcode
   */
  template <typename F, typename U = std::invoke_result_t<F>>
  Result<U, E> map(F &&func) const noexcept(noexcept(func())) {
    if (is_ok_)
      return Result<U, E>::Ok(func());
    return Result<U, E>::Err(error_);
  }

  /**
   * @brief Maps the error if Err, else propagates Ok.
   * @code
   * using MyResult = cpp_result::Result<void, std::string>;
   * auto r = MyResult::Err("fail");
   * auto mapped = r.map_err([](const std::string& e){ return e+"!"; });
   * // mapped.unwrap_err() == "fail!"
   * @endcode
   */
  template <typename F, typename E2 = std::invoke_result_t<F, E>>
  Result<void, E2> map_err(F &&func) const
      noexcept(noexcept(func(std::declval<E>()))) {
    if (is_ok_)
      return Result<void, E2>::Ok();
    return Result<void, E2>::Err(func(error_));
  }

  /**
   * @brief Chains another result-producing function if Ok, else propagates Err.
   * @code
   * using MyResult = cpp_result::Result<void, std::string>;
   * auto r = MyResult::Ok();
   * auto chained = r.and_then([]{
   *    return cpp_result::Ok<int, std::string>(43);
   * });
   * // chained.unwrap() == 43
   * @endcode
   */
  template <typename F, typename R = std::invoke_result_t<F>>
  R and_then(F &&func) const noexcept(noexcept(func())) {
    if (is_ok_)
      return func();
    return R::Err(error_);
  }

  /**
   * @brief Unwraps the value or aborts with a custom message if Err.
   * @code
   * using MyResult = cpp_result::Result<void, std::string>;
   * auto r = MyResult::Ok();
   * r.expect("should not fail");
   * @endcode
   */
  void expect(const char msg[]) const noexcept { EXPECT_OR_ABORT(is_ok_, msg); }

  /**
   * @brief Unwraps the error or aborts with a custom message if Ok.
   * @code
   * using MyResult = cpp_result::Result<void, std::string>;
   * auto r = MyResult::Err("fail");
   * std::string e = r.expect_err("should not fail"); // e == "fail"
   * @endcode
   */
  E &expect_err(const char msg[]) noexcept {
    EXPECT_OR_ABORT(!is_ok_, msg);
    return error_;
  }

  const E &expect_err(const char msg[]) const noexcept {
    EXPECT_OR_ABORT(!is_ok_, msg);
    return error_;
  }

  /**
   * @brief Calls func() if Ok, returns self.
   * @code
   * using MyResult = cpp_result::Result<void, std::string>;
   * auto r = MyResult::Ok();
   * r.inspect([]{ std::cout << "All good!"; });
   * @endcode
   */
  template <typename F>
  const Result &inspect(F &&func) const noexcept(noexcept(func())) {
    if (is_ok_)
      func();
    return *this;
  }

  /**
   * @brief Calls func(error) if Err, returns self.
   * @code
   * using MyResult = cpp_result::Result<void, std::string>;
   * auto r = MyResult::Err("fail");
   * r.inspect_err([](const std::string& e){ std::cout << e; });
   * @endcode
   */
  template <typename F>
  const Result &inspect_err(F &&func) const
      noexcept(noexcept(func(std::declval<const E &>()))) {
    if (!is_ok_)
      func(error_);
    return *this;
  }

  // Move semantics
  Result(Result &&other) noexcept(std::is_nothrow_move_constructible_v<E>)
      : is_ok_(other.is_ok_) {
    if (!is_ok_)
      new (&error_) E(std::move(other.error_));
  }

  Result &
  operator=(Result &&other) noexcept(std::is_nothrow_move_assignable_v<E>) {
    if (this != &other) {
      is_ok_ = other.is_ok_;
      if (!is_ok_)
        new (&error_) E(std::move(other.error_));
    }
    return *this;
  }

  // Conditional copy semantics
  Result(const Result &other) noexcept(std::is_nothrow_copy_constructible_v<E>)
      : is_ok_(other.is_ok_) {
    if (!is_ok_)
      new (&error_) E(other.error_);
  }

  Result &operator=(const Result &other) noexcept(
      std::is_nothrow_copy_assignable_v<E>) {
    if (this != &other) {
      is_ok_ = other.is_ok_;
      if (!is_ok_)
        new (&error_) E(other.error_);
    }
    return *this;
  }

private:
  E error_;
  bool is_ok_;

  Result() noexcept : is_ok_(true) {}
  explicit Result(E err) noexcept : is_ok_(false) {
    new (&error_) E(std::move(err));
  }
};

template <typename T, typename E> inline Result<T, E> Ok(T val) {
  return Result<T, E>::Ok(std::move(val));
}
template <typename T, typename E> inline Result<T, E> Err(E err) {
  return Result<T, E>::Err(std::move(err));
}
template <typename E> inline Result<void, E> Ok() {
  return Result<void, E>::Ok();
}
template <typename E> inline Result<void, E> Err(E err) {
  return Result<void, E>::Err(std::move(err));
}

} // namespace cpp_result
