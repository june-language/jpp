#include "Common.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

using namespace june::err;

TEST(Errs, KindAsString) {
  EXPECT_STREQ(errKindAsString(ErrorKind::None), "none");
  EXPECT_STREQ(errKindAsString(ErrorKind::FileIo), "file i/o");
  EXPECT_STREQ(errKindAsString(ErrorKind::Exec), "exec error");
  EXPECT_STREQ(errKindAsString(ErrorKind::Args), "args error");
  EXPECT_STREQ(errKindAsString(ErrorKind::Raised), "raised");
  EXPECT_STREQ(errKindAsString(ErrorKind::Unwrap), "unwrap error");
}

static auto err = Error(ErrorKind::Exec, "GenericDummy", false);

TEST(Errs, ErrStream) {
  std::stringstream ss;
  ss << err;
  EXPECT_STREQ(ss.str().c_str(), "err: GenericDummy (exec error)");
}

TEST(Errs, ErrFatal) {
  EXPECT_FALSE(err.fatal);
  err.setFatal(true);
  EXPECT_TRUE(err.fatal);

  // TODO: get death tests to work with the matcher
  EXPECT_DEATH(err.print(std::cerr), ".* - fatal error: exiting");

  err.setFatal(false);
  EXPECT_FALSE(err.fatal); // reset for the next few tests
}

using TestRes = Result<int, Error>;
static auto res_ok = TestRes::Ok(0);
static auto res_err = TestRes::Err(err);

TEST(Errs, ResultsStatus) {
  EXPECT_TRUE(res_ok.isOk());
  EXPECT_TRUE(res_err.isErr());

  EXPECT_FALSE(res_ok.isErr());
  EXPECT_FALSE(res_err.isOk());
}

TEST(Errs, ResultsUnwrap) {
  EXPECT_EQ(res_ok.unwrap(), 0);
  EXPECT_EQ(res_err.unwrapErr(), err);
}

TEST(Errs, ResultsUnwrapOr) {
  EXPECT_EQ(res_err.unwrapOr(1), 1);
  EXPECT_EQ(res_ok.unwrapOr(1), 0);
}

TEST(Errs, ResultsUnwrapOrElse) {
  EXPECT_EQ(res_err.unwrapOrElse([](const Error &err) { return 1; }), 1);
  EXPECT_EQ(res_ok.unwrapOrElse([](const Error &err) { return 1; }), 0);
}

TEST(Errs, ResultsUnwrapDeath) {
  EXPECT_DEATH(res_ok.unwrapErr(), ".*\\(unwrap error\\).*");
  EXPECT_DEATH(res_err.unwrap(), ".*\\(unwrap error\\).*");
}

TEST(Errs, ResultsStream) {
  std::stringstream ss;
  ss << res_ok;
  EXPECT_STREQ(ss.str().c_str(), "Ok(0)");
}
