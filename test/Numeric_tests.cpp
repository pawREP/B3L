#include "B3L/Numeric.h"
#include <gtest/gtest.h>

using namespace B3L;

TEST(NumericTests, NegateSigned) {
    constexpr int maxInt = (std::numeric_limits<int>::max)();
    constexpr int minInt  = (std::numeric_limits<int>::min)();

    EXPECT_EQ(Numeric::negate(maxInt), minInt + 1);
    EXPECT_EQ(Numeric::negate(minInt), maxInt);
}
