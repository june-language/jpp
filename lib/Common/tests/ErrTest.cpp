#include "Common.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

using namespace june::err;

TEST(Errs, KindAsString) {
  ASSERT_STREQ(errKindAsString(ErrorKind::None), "none");
  ASSERT_STREQ(errKindAsString(ErrorKind::FileIo), "file i/o");
  ASSERT_STREQ(errKindAsString(ErrorKind::Exec), "exec error");
  ASSERT_STREQ(errKindAsString(ErrorKind::Args), "args error");
  ASSERT_STREQ(errKindAsString(ErrorKind::Raised), "raised");
}

TEST(Errs, GenericError) {
  auto err = Error(ErrorKind::Exec, "GenericDummy", false);
  std::stringstream ss;

  ASSERT_FALSE(err.fatal);
  err.print(ss);
  ASSERT_STREQ(ss.str().c_str(), "err: GenericDummy (exec error)\n");

  err.setFatal(true);
  // TODO: get death tests to work as expected
  ASSERT_DEATH(err.print(std::cerr),
               ".*");
              //  "err: GenericDummy (exec error)\nfatal error: exiting\n");
}

TEST(Errs, Results) {
  using TestRes = Result<int, Error>;

  auto err = Error(ErrorKind::Exec, "GenericDummy", false);
  auto res_err = TestRes::Err(err);
  auto res_ok = TestRes::Ok(0);

  ASSERT_TRUE(res_ok.isOk());
  ASSERT_TRUE(res_err.isErr());

  ASSERT_FALSE(res_ok.isErr());
  ASSERT_FALSE(res_ok.isOk());

  ASSERT_EQ(res_ok.unwrap(), 0);
  ASSERT_EQ(res_err.unwrapErr(), err);

  ASSERT_EQ(res_err.unwrapOr(1), 1);
  ASSERT_EQ(res_ok.unwrapOr(1), 0);

  ASSERT_EQ(res_err.unwrapOrElse([](const Error &err) { return 1; }), 1);
  ASSERT_EQ(res_ok.unwrapOrElse([](const Error &err) { return 1; }), 0);

  // MARK: Death Tests (Results)
  ASSERT_DEATH(res_ok.unwrapErr(), ".*");
  ASSERT_DEATH(res_err.unwrap(), ".*");


  // TODO: Stream tests for Result::Err
  std::stringstream ss;
  ss << res_ok;
  ASSERT_STREQ(ss.str().c_str(), "Ok(0)");
}
