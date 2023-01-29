#include "Common.hpp"
#include <gtest/gtest.h>

TEST(StringTest, EndsWith) {
  auto s = "Hello World";

  EXPECT_TRUE(june::string::endsWith(s, "World"));
}

TEST(StringTest, StartsWith) {
  auto s = "Hello World";

  EXPECT_TRUE(june::string::startsWith(s, "Hello"));
}

TEST(StringTest, Split) {
  auto s = "Hello World";

  auto result = june::string::split(s, " ");

  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], "Hello");
  EXPECT_EQ(result[1], "World");
}

TEST(StringTest, ReplaceAll) {
  auto s = "Hello World";

  auto result = june::string::replaceAll(s, "Hello", "Goodbye");

  EXPECT_EQ(result, "Goodbye World");
}

TEST(StringTest, Join) {
  auto strs = std::vector<std::string>{"Hello", "World"};

  auto result = june::string::join(strs, " ");

  EXPECT_EQ(result, "Hello World");
}

TEST(StringTest, ToLower) {
  auto s = "Hello World";

  auto result = june::string::toLower(s);

  EXPECT_EQ(result, "hello world");
}

TEST(StringTest, ToUpper) {
  auto s = "Hello World";

  auto result = june::string::toUpper(s);

  EXPECT_EQ(result, "HELLO WORLD");
}

TEST(StringTest, ToTitle) {
  auto s = "Hello World";

  auto result = june::string::toTitle(s);

  EXPECT_EQ(result, "Hello World");
}

TEST(StringTest, Trim) {
  auto s = " Hello World ";

  auto result = june::string::trim(s);

  EXPECT_EQ(result, "Hello World");

  // Newlines/Tabs trimming

  s = "\tHello World\n";

  result = june::string::trim(s);

  EXPECT_EQ(result, "Hello World");
}
