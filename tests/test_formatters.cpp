#pragma warning(push)
#pragma warning(disable : 4668)
#include <gtest/gtest.h>
#pragma warning(pop)

#include <QString>
#include <vector>

#include <uibase/formatters.h>

#include <format>

TEST(FormatterTest, Enum)
{
  enum class E
  {
    a = 1,
    b = 2
  };
  ASSERT_EQ("1", std::format("{}", E::a));
  ASSERT_EQ("2", std::format("{}", E::b));
}

// simply test that we can format between string types
TEST(FormatterTest, String)
{
  using namespace std::string_literals;
  ASSERT_EQ("Hello World!", std::format("{}", L"Hello World!"s));
  ASSERT_EQ(L"Hello World!", std::format(L"{}", "Hello World!"s));
  ASSERT_EQ("Hello World!", std::format("{}", QString("Hello World!")));
  ASSERT_EQ(L"Hello World!", std::format(L"{}", QString("Hello World!")));
}

TEST(FormatterTest, RandomAccessContainer)
{
  ASSERT_EQ("[]", std::format("{}", std::vector<int>{}));
  ASSERT_EQ("()", std::format("{:b()}", std::vector<int>{}));
  ASSERT_EQ("{}", std::format("{:b{}}", std::vector<int>{}));

  ASSERT_EQ("[ 1, 2, 3 ]", std::format("{}", std::vector{1, 2, 3}));
  ASSERT_EQ("(1, 2, 3)", std::format("{:b()}", std::vector{1, 2, 3}));
  ASSERT_EQ("( 1, 2, 3 )", std::format("{:b( )}", std::vector{1, 2, 3}));
  ASSERT_EQ("{ 1, 2, 3 }", std::format("{:b{ }}", std::vector{1, 2, 3}));
  ASSERT_EQ("{ 1, 2, 3 }", std::format("{:b{ }$}", std::vector{1, 2, 3}));

  ASSERT_EQ("[ 1 ; 2 ; 3 ]", std::format("{:d ; $}", std::vector{1, 2, 3}));
  ASSERT_EQ("{ 1 ; 2 ; 3 }", std::format("{:b{ }d ; $}", std::vector{1, 2, 3}));

  ASSERT_EQ("[ 1, 2, ..., 7 ]", std::format("{}", std::vector{1, 2, 3, 4, 5, 6, 7}));
  ASSERT_EQ("[ 1, 2, ..., 6, 7 ]",
            std::format("{:n4}", std::vector{1, 2, 3, 4, 5, 6, 7}));
  ASSERT_EQ("[ 1, 2, 3, 4, 5, 6, 7 ]",
            std::format("{:n50}", std::vector{1, 2, 3, 4, 5, 6, 7}));
  ASSERT_EQ("( 1 / 2 / 3 / ... / 6 / 7 )",
            std::format("{:n5$d / $b( )}", std::vector{1, 2, 3, 4, 5, 6, 7}));

  ASSERT_EQ("[ AL, Holt59, Silarn ]",
            std::format("{}", QStringList{"AL", "Holt59", "Silarn"}));

  ASSERT_EQ("[ QVariant(type=QString, value=MO2), QVariant(type=bool, value=true), "
            "QVariant(type=int, value=45) ]",
            std::format("{}", QVariantList{"MO2", true, 45}));
}
