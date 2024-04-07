#include "gtest/gtest.h"
#include "util/StringUtils.hpp"

TEST(StringUtilsTests, trim) {
    std::string str = "  abcde  ";
    auto result = wall::StringUtils::trim(str);
    std::string expected = "abcde";
    EXPECT_EQ(result, expected);

    str = "abcde  ";
    result = wall::StringUtils::trim(str);
    expected = "abcde";
    EXPECT_EQ(result, expected);

    str = "  abcde";
    result = wall::StringUtils::trim(str);
    expected = "abcde";
    EXPECT_EQ(result, expected);

    str = "abcde";
    result = wall::StringUtils::trim(str);
    expected = "abcde";
    EXPECT_EQ(result, expected);

    str = "  ";
    result = wall::StringUtils::trim(str);
    expected = "";
    EXPECT_EQ(result, expected);

    str = "";
    result = wall::StringUtils::trim(str);
    expected = "";
    EXPECT_EQ(result, expected);
}

TEST(StringUtilsTests, split) {
    std::string str = "a,b,c,d,e";
    auto result = wall::StringUtils::split(str, ',');
    std::vector<std::string> expected = {"a", "b", "c", "d", "e"};
    EXPECT_EQ(result, expected);

    str = "a";
    result = wall::StringUtils::split(str, ',');
    expected = {"a"};
    EXPECT_EQ(result, expected);
}

TEST(StringUtilsTests, splitBad) {
    std::string str = "abcde";
    auto result = wall::StringUtils::split(str, ',');
    std::vector<std::string> expected = {"abcde"};
    EXPECT_EQ(result, expected);

    str = "a,b,c,d,e,";
    result = wall::StringUtils::split(str, ',');
    expected = {"a", "b", "c", "d", "e", ""};
    EXPECT_EQ(result, expected);

    str = ",a,b,c,d,e";
    result = wall::StringUtils::split(str, ',');
    expected = {"", "a", "b", "c", "d", "e"};
    EXPECT_EQ(result, expected);

    str = ",a,b,c,d,e,";
    result = wall::StringUtils::split(str, ',');
    expected = {"", "a", "b", "c", "d", "e", ""};
    EXPECT_EQ(result, expected);

    str = "";
    result = wall::StringUtils::split(str, ',');
    expected = {""};
    EXPECT_EQ(result, expected);

    str = ",";
    result = wall::StringUtils::split(str, ',');
    expected = {"", ""};
    EXPECT_EQ(result, expected);

    str = ",,";
    result = wall::StringUtils::split(str, ',');
    expected = {"", "", ""};
    EXPECT_EQ(result, expected);
}

TEST(StringUtilsTests, split_and_trim) {
    std::string str = "a, b, c, d, e";
    auto result = wall::StringUtils::split_and_trim(str, ',');
    std::vector<std::string> expected = {"a", "b", "c", "d", "e"};
    EXPECT_EQ(result, expected);

    str = "a";
    result = wall::StringUtils::split_and_trim(str, ',');
    expected = {"a"};
    EXPECT_EQ(result, expected);

    str = "a, b, c, d, e, ";
    result = wall::StringUtils::split_and_trim(str, ',');
    expected = {"a", "b", "c", "d", "e", ""};
    EXPECT_EQ(result, expected);

    str = "a, b, c, d, e,";
    result = wall::StringUtils::split_and_trim(str, ',');
    expected = {"a", "b", "c", "d", "e", ""};
    EXPECT_EQ(result, expected);

    str = "a, b, c, d, e, ,";
    result = wall::StringUtils::split_and_trim(str, ',');
    expected = {"a", "b", "c", "d", "e", "", ""};
    EXPECT_EQ(result, expected);

    str = ", a, b, c, d, e";
    result = wall::StringUtils::split_and_trim(str, ',');
    expected = {"", "a", "b", "c", "d", "e"};
    EXPECT_EQ(result, expected);

    str = ", a, b, c, d, e,";
    result = wall::StringUtils::split_and_trim(str, ',');
    expected = {"", "a", "b", "c", "d", "e", ""};
    EXPECT_EQ(result, expected);

    str = ", a, b, c, d, e, ,";
    result = wall::StringUtils::split_and_trim(str, ',');
    expected = {"", "a", "b", "c", "d", "e", "", ""};
    EXPECT_EQ(result, expected);

    str = "";
    result = wall::StringUtils::split_and_trim(str, ',');
    expected = {""};
    EXPECT_EQ(result, expected);

    str = ",";
    result = wall::StringUtils::split_and_trim(str, ',');
    expected = {"", ""};
    EXPECT_EQ(result, expected);
}
