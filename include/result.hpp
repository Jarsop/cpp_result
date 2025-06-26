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

template <typename T, typename E> class [[nodiscard]] Result {
  static_assert(!std::is_reference_v<T> && !std::is_reference_v<E>,
                "Result<T,E> does not support reference types");

public:
  static inline Result Ok(T val) noexcept { return Result(std::move(val)); }
  static inline Result Err(E err) noexcept { return Result(std::move(err)); }

  constexpr bool is_ok() const noexcept { return is_ok_; }
  constexpr bool is_err() const noexcept { return !is_ok_; }

  inline T &unwrap() noexcept {
    EXPECT_OR_ABORT(is_ok_, "unwrap called on Result::Err()");
    return data_.value;
  }
  inline const T &unwrap() const noexcept {
    EXPECT_OR_ABORT(is_ok_, "unwrap called on Result::Err()");
    return data_.value;
  }

  inline E &unwrap_err() noexcept {
    EXPECT_OR_ABORT(!is_ok_, "unwrap_err called on Result::Ok()");
    return data_.error;
  }
  inline const E &unwrap_err() const noexcept {
    EXPECT_OR_ABORT(!is_ok_, "unwrap_err called on Result::Ok()");
    return data_.error;
  }

  inline T unwrap_or(T default_value) const noexcept {
    return is_ok_ ? data_.value : default_value;
  }

  template <typename F>
  inline T unwrap_or_else(F &&func) const noexcept(noexcept(func())) {
    return is_ok_ ? data_.value : std::forward<F>(func)();
  }

  template <typename F, typename U = std::invoke_result_t<F, T>>
  Result<U, E> map(F &&func) const noexcept(noexcept(func(std::declval<T>()))) {
    if (is_ok_)
      return Result<U, E>::Ok(func(data_.value));
    return Result<U, E>::Err(data_.error);
  }

  template <typename F, typename E2 = std::invoke_result_t<F, E>>
  Result<T, E2> map_err(F &&func) const
      noexcept(noexcept(func(std::declval<E>()))) {
    if (is_ok_)
      return Result<T, E2>::Ok(data_.value);
    return Result<T, E2>::Err(func(data_.error));
  }

  template <typename F, typename R = typename std::invoke_result_t<F, T>>
  R and_then(F &&func) const noexcept(noexcept(func(std::declval<T>()))) {
    if (is_ok_)
      return func(data_.value);
    return R::Err(data_.error);
  }

  T &expect(const char *msg) noexcept {
    EXPECT_OR_ABORT(is_ok_, msg);
    return data_.value;
  }

  const T &expect(const char *msg) const noexcept {
    EXPECT_OR_ABORT(is_ok_, msg);
    return data_.value;
  }

  E &expect_err(const char *msg) noexcept {
    EXPECT_OR_ABORT(!is_ok_, msg);
    return data_.error;
  }

  const E &expect_err(const char *msg) const noexcept {
    EXPECT_OR_ABORT(!is_ok_, msg);
    return data_.error;
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

// Partial specialization for Result<void, E>
template <typename E> class [[nodiscard]] Result<void, E> {
  static_assert(!std::is_reference_v<E>,
                "Result<void,E> does not support reference types");

public:
  static inline Result Ok() noexcept { return Result(); }
  static inline Result Err(E err) noexcept { return Result(std::move(err)); }

  inline bool is_ok() const noexcept { return is_ok_; }
  inline bool is_err() const noexcept { return !is_ok_; }

  inline void unwrap() noexcept {
    EXPECT_OR_ABORT(is_ok_, "unwrap called on Result::Err()");
  }

  inline E &unwrap_err() noexcept {
    EXPECT_OR_ABORT(!is_ok_, "unwrap_err called on Result::Ok()");
    return error_;
  }
  inline const E &unwrap_err() const noexcept {
    EXPECT_OR_ABORT(!is_ok_, "unwrap_err called on Result::Ok()");
    return error_;
  }

  template <typename F, typename U = std::invoke_result_t<F>>
  Result<U, E> map(F &&func) const noexcept(noexcept(func())) {
    if (is_ok_)
      return Result<U, E>::Ok(func());
    return Result<U, E>::Err(error_);
  }

  template <typename F, typename E2 = std::invoke_result_t<F, E>>
  Result<void, E2> map_err(F &&func) const
      noexcept(noexcept(func(std::declval<E>()))) {
    if (is_ok_)
      return Result<void, E2>::Ok();
    return Result<void, E2>::Err(func(error_));
  }

  template <typename F, typename R = std::invoke_result_t<F>>
  R and_then(F &&func) const noexcept(noexcept(func())) {
    if (is_ok_)
      return func();
    return R::Err(error_);
  }

  void expect(const char msg[]) const noexcept { EXPECT_OR_ABORT(is_ok_, msg); }

  E &expect_err(const char msg[]) noexcept {
    EXPECT_OR_ABORT(!is_ok_, msg);
    return error_;
  }

  const E &expect_err(const char msg[]) const noexcept {
    EXPECT_OR_ABORT(!is_ok_, msg);
    return error_;
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

} // namespace cpp_result
