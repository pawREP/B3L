#include "B3L/StringUtil.h"
#include <gtest/gtest.h>

const char* whitespaceTestStr = " \t\r\n\v\f\r a\tb\nc\v d\fe\rf ";

TEST(StringUtilTests, removeWhitespace) {
    std::string test = whitespaceTestStr;

    B3L::StringUtil::removeWhitespace(test);

    EXPECT_TRUE(test.compare("abcdef") == 0);
}

TEST(StringUtilTests, trim) {
    std::string test = whitespaceTestStr;

    B3L::StringUtil::trim(test);
}

TEST(StringUtilTests, compareCaseInsensitive) {
    std::string str0 = "TEstStr";
    std::string str1 = "Teststr";

    EXPECT_TRUE(B3L::StringUtil::iequal(str0, str1));
}

TEST(StringUtilTests, replace) {
    std::string str = "I like cats and dogs but I prefer cats over dogs";

    B3L::StringUtil::replace(str, "dog", "cat");

    EXPECT_TRUE(str.compare("I like cats and cats but I prefer cats over cats") == 0);
}