#include "B3L/AOBScanner.h"
#include <gtest/gtest.h>

using namespace B3L;

TEST(AOBScannerTests, PatternFromString) {
    // Good
    EXPECT_TRUE(AOBPattern::fromString("01 23 45 67 89 AB CD EF ??"));
    EXPECT_TRUE(AOBPattern::fromString("0123456789ABCDEF??"));
    EXPECT_TRUE(AOBPattern::fromString("0123456789abcdef??"));

    // Bad
    EXPECT_FALSE(AOBPattern::fromString(""));          // Empty string is not a valid pattern
    EXPECT_FALSE(AOBPattern::fromString("12 3"));      // Str len not even
    EXPECT_FALSE(AOBPattern::fromString("1? ?2"));     // Bad wildcard offset
    EXPECT_FALSE(AOBPattern::fromString("12 3H"));     // Bad symbol
    EXPECT_FALSE(AOBPattern::fromString("0x11 0x32")); // 0x syntax not supported
}

TEST(AOBScannerTests, Find) {
    std::vector<uint8_t> haystack{ 0x8B, 0x0C, 0x08, 0xE8, 0x74, 0xED, 0xED, 0xFF, 0x48, 0x89 };
    AOBPattern needle = AOBPattern::fromString("ED ?? FF 48 89 ").value();

    auto it = AOBScanner::find(haystack.begin(), haystack.end(), needle);

    auto offset = std::distance(haystack.begin(), it);

    EXPECT_EQ(offset, 5);
}
