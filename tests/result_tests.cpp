#include <gtest/gtest.h>
#include <result.hpp>
#include <string>

struct Error {
  std::string message;
  bool operator==(const Error &other) const { return message == other.message; }
};

template <typename T> using Result = cpp_result::Result<T, Error>;

template <typename T> inline Result<T> Ok(T value) {
  return Result<T>::Ok(std::forward<T>(value));
}
template <typename T> inline Result<T> Err(Error err) {
  return Result<T>::Err(std::move(err));
}

TEST(ResultTest, OkValue) {
  auto res = Ok<int>(10);
  EXPECT_TRUE(res.is_ok());
  EXPECT_EQ(res.unwrap(), 10);
}

TEST(ResultTest, ErrValue) {
  auto res = Err<int>({"error"});
  EXPECT_TRUE(res.is_err());
  EXPECT_EQ(res.unwrap_err().message, "error");
}

TEST(ResultTest, UnwrapOr) {
  auto res = Err<int>({"fail"});
  EXPECT_EQ(res.unwrap_or(42), 42);
}

TEST(ResultTest, UnwrapOrElse) {
  auto res = Err<int>({"fail"});
  EXPECT_EQ(res.unwrap_or_else([]() { return 99; }), 99);
}

TEST(ResultTest, MoveSemantics) {
  auto res = Ok<std::string>("hello");
  auto moved = std::move(res);
  EXPECT_TRUE(moved.is_ok());
  EXPECT_EQ(moved.unwrap(), "hello");
}

TEST(ResultTest, CopySemantics) {
  auto res = Ok<int>(5);
  auto copy = res;
  EXPECT_TRUE(copy.is_ok());
  EXPECT_EQ(copy.unwrap(), 5);
}

TEST(ResultTest, VoidOk) {
  auto res = cpp_result::Ok<Error>();
  EXPECT_TRUE(res.is_ok());
  EXPECT_NO_THROW(res.unwrap());
}

TEST(ResultTest, VoidErr) {
  auto res = cpp_result::Err<Error>({"void error"});
  EXPECT_TRUE(res.is_err());
  EXPECT_EQ(res.unwrap_err().message, "void error");
}

TEST(ResultTest, Map) {
  auto res = Ok<int>(2);
  auto mapped = res.map([](int v) { return v * 10; });
  EXPECT_TRUE(mapped.is_ok());
  EXPECT_EQ(mapped.unwrap(), 20);
  auto err = Err<int>({"fail"});
  auto mapped_err = err.map([](int v) { return v * 10; });
  EXPECT_TRUE(mapped_err.is_err());
  EXPECT_EQ(mapped_err.unwrap_err().message, "fail");
}

TEST(ResultTest, MapErr) {
  auto res = Err<int>({"fail"});
  auto mapped =
      res.map_err([](const Error &e) { return Error{"mapped: " + e.message}; });
  EXPECT_TRUE(mapped.is_err());
  EXPECT_EQ(mapped.unwrap_err().message, "mapped: fail");
  auto ok = Ok<int>(1);
  auto mapped_ok =
      ok.map_err([](const Error &) { return Error{"should not happen"}; });
  EXPECT_TRUE(mapped_ok.is_ok());
  EXPECT_EQ(mapped_ok.unwrap(), 1);
}

TEST(ResultTest, AndThen) {
  auto res = Ok<int>(3);
  auto chained =
      res.and_then([](int v) { return Ok<std::string>(std::to_string(v)); });
  EXPECT_TRUE(chained.is_ok());
  EXPECT_EQ(chained.unwrap(), "3");
  auto err = Err<int>({"fail"});
  auto chained_err =
      err.and_then([](int v) { return Ok<std::string>(std::to_string(v)); });
  EXPECT_TRUE(chained_err.is_err());
  EXPECT_EQ(chained_err.unwrap_err().message, "fail");
}

TEST(ResultTest, VoidMap) {
  auto ok = cpp_result::Ok<Error>();
  auto mapped = ok.map([]() { return 42; });
  EXPECT_TRUE(mapped.is_ok());
  EXPECT_EQ(mapped.unwrap(), 42);
  auto err = cpp_result::Err<Error>({"void fail"});
  auto mapped_err = err.map([]() { return 42; });
  EXPECT_TRUE(mapped_err.is_err());
  EXPECT_EQ(mapped_err.unwrap_err().message, "void fail");
}

TEST(ResultTest, VoidMapErr) {
  auto err = cpp_result::Err<Error>({"void fail"});
  auto mapped = err.map_err(
      [](const Error &e) { return Error{"void mapped: " + e.message}; });
  EXPECT_TRUE(mapped.is_err());
  EXPECT_EQ(mapped.unwrap_err().message, "void mapped: void fail");
  auto ok = cpp_result::Ok<Error>();
  auto mapped_ok =
      ok.map_err([](const Error &) { return Error{"should not happen"}; });
  EXPECT_TRUE(mapped_ok.is_ok());
}

TEST(ResultTest, VoidAndThen) {
  auto ok = cpp_result::Ok<Error>();
  auto chained = ok.and_then([]() { return Ok<std::string>("side effect"); });
  EXPECT_TRUE(chained.is_ok());
  EXPECT_EQ(chained.unwrap(), "side effect");
  auto err = cpp_result::Err<Error>({"void fail"});
  auto chained_err =
      err.and_then([]() { return Ok<std::string>("side effect"); });
  EXPECT_TRUE(chained_err.is_err());
  EXPECT_EQ(chained_err.unwrap_err().message, "void fail");
}

TEST(ResultTest, UnwrapDeathOnErr) {
  auto res = Err<int>({"fail"});
  EXPECT_DEATH(res.unwrap(), "unwrap called on Result::Err()");
}

TEST(ResultTest, UnwrapErrDeathOnOk) {
  auto res = Ok<int>(123);
  EXPECT_DEATH(res.unwrap_err(), "unwrap_err called on Result::Ok()");
}

TEST(ResultTest, ExpectReturnsValueOnOk) {
  auto res = Ok<int>(42);
  EXPECT_EQ(res.expect("should not fail"), 42);
}

TEST(ResultTest, ExpectDeathOnErr) {
  auto res = Err<int>({"fail"});
  EXPECT_DEATH(res.expect("custom error message"), "custom error message");
}

TEST(ResultTest, ExpectErrReturnsErrorOnErr) {
  auto res = Err<int>({"fail"});
  EXPECT_EQ(res.expect_err("should not fail").message, "fail");
}

TEST(ResultTest, ExpectErrDeathOnOk) {
  auto res = Ok<int>(123);
  EXPECT_DEATH(res.expect_err("custom error message"), "custom error message");
}

TEST(ResultTest, VoidExpectDeathOnErr) {
  auto res = cpp_result::Err<Error>({"fail"});
  EXPECT_DEATH(res.expect("void error"), "void error");
}

TEST(ResultTest, VoidExpectErrDeathOnOk) {
  auto res = cpp_result::Ok<Error>();
  EXPECT_DEATH(res.expect_err("should fail"), "should fail");
}

TEST(ResultTest, InspectOk) {
  auto res = Ok<int>(42);
  int called = 0;
  res.inspect([&](const int &v) {
    EXPECT_EQ(v, 42);
    called = 1;
  });
  EXPECT_EQ(called, 1);
}

TEST(ResultTest, InspectErrNoCall) {
  auto res = Err<int>({"fail"});
  int called = 0;
  res.inspect([&](const int &) { called = 1; });
  EXPECT_EQ(called, 0);
}

TEST(ResultTest, InspectErr) {
  auto res = Err<int>({"fail"});
  int called = 0;
  res.inspect_err([&](const Error &e) {
    EXPECT_EQ(e.message, "fail");
    called = 1;
  });
  EXPECT_EQ(called, 1);
}

TEST(ResultTest, InspectOkNoCall) {
  auto res = Ok<int>(42);
  int called = 0;
  res.inspect_err([&](const Error &) { called = 1; });
  EXPECT_EQ(called, 0);
}

TEST(ResultTest, VoidInspectOk) {
  auto ok = cpp_result::Ok<Error>();
  int called = 0;
  ok.inspect([&]() { called = 1; });
  EXPECT_EQ(called, 1);
}

TEST(ResultTest, VoidInspectErrNoCall) {
  auto err = cpp_result::Err<Error>({"fail"});
  int called = 0;
  err.inspect([&]() { called = 1; });
  EXPECT_EQ(called, 0);
}

TEST(ResultTest, VoidInspectErr) {
  auto err = cpp_result::Err<Error>({"fail"});
  int called = 0;
  err.inspect_err([&](const Error &e) {
    EXPECT_EQ(e.message, "fail");
    called = 1;
  });
  EXPECT_EQ(called, 1);
}

TEST(ResultTest, VoidInspectOkNoCall) {
  auto ok = cpp_result::Ok<Error>();
  int called = 0;
  ok.inspect_err([&](const Error &) { called = 1; });
  EXPECT_EQ(called, 0);
}

TEST(ResultTest, UnwrapOrDefault) {
  using MyResult = cpp_result::Result<int, std::string>;
  MyResult ok = MyResult::Ok(42);
  MyResult err = MyResult::Err("fail");
  EXPECT_EQ(ok.unwrap_or_default(), 42);
  EXPECT_EQ(err.unwrap_or_default(), 0); // int default

  struct Dummy {
    int x = 123;
    Dummy() = default;
    Dummy(int v) : x(v) {}
    bool operator==(const Dummy &other) const { return x == other.x; }
  };
  cpp_result::Result<Dummy, std::string> ok2 =
      cpp_result::Ok<Dummy, std::string>(Dummy(7));
  cpp_result::Result<Dummy, std::string> err2 =
      cpp_result::Err<Dummy, std::string>("fail");
  EXPECT_EQ(ok2.unwrap_or_default(), Dummy(7));
  EXPECT_EQ(err2.unwrap_or_default(), Dummy());
}

TEST(ResultTest, IsOkAnd_IsErrAnd) {
  auto ok = Ok<int>(42);
  auto err = Err<int>({"fail"});
  EXPECT_TRUE(ok.is_ok_and([](int v) { return v == 42; }));
  EXPECT_FALSE(ok.is_ok_and([](int v) { return v == 0; }));
  EXPECT_FALSE(err.is_ok_and([](int) { return true; }));
  EXPECT_TRUE(
      err.is_err_and([](const Error &e) { return e.message == "fail"; }));
  EXPECT_FALSE(
      err.is_err_and([](const Error &e) { return e.message == "nope"; }));
  EXPECT_FALSE(ok.is_err_and([](const Error &) { return true; }));
}

TEST(ResultTest, OkErrOption) {
  auto ok = Ok<int>(42);
  auto err = Err<int>({"fail"});
  EXPECT_TRUE(ok.ok().has_value());
  EXPECT_EQ(ok.ok().value(), 42);
  EXPECT_FALSE(err.ok().has_value());
  EXPECT_TRUE(err.err().has_value());
  EXPECT_EQ(err.err()->message, "fail");
  EXPECT_FALSE(ok.err().has_value());
}

TEST(ResultTest, AndOr) {
  auto ok1 = Ok<int>(1);
  auto ok2 = Ok<int>(2);
  auto err1 = Err<int>({"fail"});
  auto out1 = ok1.and_(ok2);
  EXPECT_TRUE(out1.is_ok());
  EXPECT_EQ(out1.unwrap(), 2);
  auto out2 = err1.and_(ok2);
  EXPECT_TRUE(out2.is_err());
  EXPECT_EQ(out2.unwrap_err().message, "fail");
  auto out3 = err1.or_(ok2);
  EXPECT_TRUE(out3.is_ok());
  EXPECT_EQ(out3.unwrap(), 2);
  auto out4 = ok1.or_(ok2);
  EXPECT_TRUE(out4.is_ok());
  EXPECT_EQ(out4.unwrap(), 1);
}

TEST(ResultTest, OrElse) {
  auto err = Err<int>({"fail"});
  auto ok = Ok<int>(42);
  auto out = err.or_else([] { return Ok<int>(123); });
  EXPECT_TRUE(out.is_ok());
  EXPECT_EQ(out.unwrap(), 123);
  auto out2 = ok.or_else([] { return Ok<int>(0); });
  EXPECT_TRUE(out2.is_ok());
  EXPECT_EQ(out2.unwrap(), 42);
}

TEST(ResultTest, MapOr_MapOrElse) {
  auto ok = Ok<int>(21);
  auto err = Err<int>({"fail"});
  EXPECT_EQ(ok.map_or(0, [](int v) { return v * 2; }), 42);
  EXPECT_EQ(err.map_or(0, [](int v) { return v * 2; }), 0);
  EXPECT_EQ(ok.map_or_else([] { return 0; }, [](int v) { return v * 2; }), 42);
  EXPECT_EQ(err.map_or_else([] { return 0; }, [](int v) { return v * 2; }), 0);
}

TEST(ResultTest, Contains_ContainsErr) {
  auto ok = Ok<int>(42);
  auto err = Err<int>({"fail"});
  EXPECT_TRUE(ok.contains(42));
  EXPECT_FALSE(ok.contains(0));
  EXPECT_FALSE(err.contains(42));
  EXPECT_TRUE(err.contains_err(Error{"fail"}));
  EXPECT_FALSE(err.contains_err(Error{"nope"}));
  EXPECT_FALSE(ok.contains_err(Error{"fail"}));
}

TEST(ResultTest, Flatten) {
  using Inner = cpp_result::Result<int, std::string>;
  using Outer = cpp_result::Result<Inner, std::string>;
  Outer okok = Outer::Ok(Inner::Ok(42));
  Outer okerr = Outer::Ok(Inner::Err("fail"));
  Outer err = Outer::Err("outer fail");
  auto flat1 = okok.flatten();
  auto flat2 = okerr.flatten();
  auto flat3 = err.flatten();
  EXPECT_TRUE(flat1.is_ok());
  EXPECT_EQ(flat1.unwrap(), 42);
  EXPECT_TRUE(flat2.is_err());
  EXPECT_EQ(flat2.unwrap_err(), "fail");
  EXPECT_TRUE(flat3.is_err());
  EXPECT_EQ(flat3.unwrap_err(), "outer fail");
}
