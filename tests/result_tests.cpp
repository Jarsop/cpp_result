#include <gtest/gtest.h>
#include <result.hpp>
#include <string>

struct Error {
  std::string message;
};

template <typename T> using TestResult = cpp_result::Result<T, Error>;
using VoidResult = cpp_result::Result<void, Error>;

TEST(ResultTest, OkValue) {
  auto res = TestResult<int>::Ok(10);
  EXPECT_TRUE(res.is_ok());
  EXPECT_EQ(res.unwrap(), 10);
}

TEST(ResultTest, ErrValue) {
  auto res = TestResult<int>::Err({"error"});
  EXPECT_TRUE(res.is_err());
  EXPECT_EQ(res.unwrap_err().message, "error");
}

TEST(ResultTest, UnwrapOr) {
  auto res = TestResult<int>::Err({"fail"});
  EXPECT_EQ(res.unwrap_or(42), 42);
}

TEST(ResultTest, UnwrapOrElse) {
  auto res = TestResult<int>::Err({"fail"});
  EXPECT_EQ(res.unwrap_or_else([]() { return 99; }), 99);
}

TEST(ResultTest, MoveSemantics) {
  auto res = TestResult<std::string>::Ok("hello");
  auto moved = std::move(res);
  EXPECT_TRUE(moved.is_ok());
  EXPECT_EQ(moved.unwrap(), "hello");
}

TEST(ResultTest, CopySemantics) {
  auto res = TestResult<int>::Ok(5);
  auto copy = res;
  EXPECT_TRUE(copy.is_ok());
  EXPECT_EQ(copy.unwrap(), 5);
}

TEST(ResultTest, VoidOk) {
  VoidResult res = VoidResult::Ok();
  EXPECT_TRUE(res.is_ok());
  EXPECT_NO_THROW(res.unwrap());
}

TEST(ResultTest, VoidErr) {
  VoidResult res = VoidResult::Err({"void error"});
  EXPECT_TRUE(res.is_err());
  EXPECT_EQ(res.unwrap_err().message, "void error");
}

TEST(ResultTest, Map) {
  auto res = TestResult<int>::Ok(2);
  auto mapped = res.map([](int v) { return v * 10; });
  EXPECT_TRUE(mapped.is_ok());
  EXPECT_EQ(mapped.unwrap(), 20);
  auto err = TestResult<int>::Err({"fail"});
  auto mapped_err = err.map([](int v) { return v * 10; });
  EXPECT_TRUE(mapped_err.is_err());
  EXPECT_EQ(mapped_err.unwrap_err().message, "fail");
}

TEST(ResultTest, MapErr) {
  auto res = TestResult<int>::Err({"fail"});
  auto mapped =
      res.map_err([](const Error &e) { return Error{"mapped: " + e.message}; });
  EXPECT_TRUE(mapped.is_err());
  EXPECT_EQ(mapped.unwrap_err().message, "mapped: fail");
  auto ok = TestResult<int>::Ok(1);
  auto mapped_ok =
      ok.map_err([](const Error &) { return Error{"should not happen"}; });
  EXPECT_TRUE(mapped_ok.is_ok());
  EXPECT_EQ(mapped_ok.unwrap(), 1);
}

TEST(ResultTest, AndThen) {
  auto res = TestResult<int>::Ok(3);
  auto chained = res.and_then(
      [](int v) { return TestResult<std::string>::Ok(std::to_string(v)); });
  EXPECT_TRUE(chained.is_ok());
  EXPECT_EQ(chained.unwrap(), "3");
  auto err = TestResult<int>::Err({"fail"});
  auto chained_err = err.and_then(
      [](int v) { return TestResult<std::string>::Ok(std::to_string(v)); });
  EXPECT_TRUE(chained_err.is_err());
  EXPECT_EQ(chained_err.unwrap_err().message, "fail");
}

TEST(ResultTest, VoidMap) {
  VoidResult ok = VoidResult::Ok();
  auto mapped = ok.map([]() { return 42; });
  EXPECT_TRUE(mapped.is_ok());
  EXPECT_EQ(mapped.unwrap(), 42);
  VoidResult err = VoidResult::Err({"void fail"});
  auto mapped_err = err.map([]() { return 42; });
  EXPECT_TRUE(mapped_err.is_err());
  EXPECT_EQ(mapped_err.unwrap_err().message, "void fail");
}

TEST(ResultTest, VoidMapErr) {
  VoidResult err = VoidResult::Err({"void fail"});
  auto mapped = err.map_err(
      [](const Error &e) { return Error{"void mapped: " + e.message}; });
  EXPECT_TRUE(mapped.is_err());
  EXPECT_EQ(mapped.unwrap_err().message, "void mapped: void fail");
  VoidResult ok = VoidResult::Ok();
  auto mapped_ok =
      ok.map_err([](const Error &) { return Error{"should not happen"}; });
  EXPECT_TRUE(mapped_ok.is_ok());
}

TEST(ResultTest, VoidAndThen) {
  VoidResult ok = VoidResult::Ok();
  auto chained =
      ok.and_then([]() { return TestResult<std::string>::Ok("side effect"); });
  EXPECT_TRUE(chained.is_ok());
  EXPECT_EQ(chained.unwrap(), "side effect");
  VoidResult err = VoidResult::Err({"void fail"});
  auto chained_err =
      err.and_then([]() { return TestResult<std::string>::Ok("side effect"); });
  EXPECT_TRUE(chained_err.is_err());
  EXPECT_EQ(chained_err.unwrap_err().message, "void fail");
}

TEST(ResultTest, UnwrapDeathOnErr) {
  auto res = TestResult<int>::Err({"fail"});
  EXPECT_DEATH(res.unwrap(), "unwrap called on Result::Err()");
}

TEST(ResultTest, UnwrapErrDeathOnOk) {
  auto res = TestResult<int>::Ok(123);
  EXPECT_DEATH(res.unwrap_err(), "unwrap_err called on Result::Ok()");
}

TEST(ResultTest, ExpectReturnsValueOnOk) {
  auto res = TestResult<int>::Ok(42);
  EXPECT_EQ(res.expect("should not fail"), 42);
}

TEST(ResultTest, ExpectDeathOnErr) {
  auto res = TestResult<int>::Err({"fail"});
  EXPECT_DEATH(res.expect("custom error message"), "custom error message");
}

TEST(ResultTest, ExpectErrReturnsErrorOnErr) {
  auto res = TestResult<int>::Err({"fail"});
  EXPECT_EQ(res.expect_err("should not fail").message, "fail");
}

TEST(ResultTest, ExpectErrDeathOnOk) {
  auto res = TestResult<int>::Ok(123);
  EXPECT_DEATH(res.expect_err("custom error message"), "custom error message");
}

TEST(ResultTest, VoidExpectDeathOnErr) {
  VoidResult res = VoidResult::Err({"fail"});
  EXPECT_DEATH(res.expect("void error"), "void error");
}

TEST(ResultTest, VoidExpectErrDeathOnOk) {
  VoidResult res = VoidResult::Ok();
  EXPECT_DEATH(res.expect_err("should fail"), "should fail");
}
