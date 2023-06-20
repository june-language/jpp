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
