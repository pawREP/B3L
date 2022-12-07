#include "B3L/Cast.h"
#include <gtest/gtest.h>
#include <limits>

using namespace B3L;

TEST(CastTests, SignCast) {
    constexpr auto signedMax        = scast<unsigned int>((std::numeric_limits<int>::max)());
    constexpr auto signedMaxPlusOne = signedMax + 1;

    EXPECT_EQ(sign_cast(signedMax), scast<int>(signedMax));
    EXPECT_THROW(sign_cast(signedMaxPlusOne), std::domain_error);
}

TEST(CastTests, DomainCast) {
    constexpr auto charMax        = scast<size_t>((std::numeric_limits<uint8_t>::max)());
    constexpr auto charMaxPlusOne = charMax + 1;

    EXPECT_EQ(domain_cast<uint8_t>(charMax), scast<uint8_t>(charMax));
    EXPECT_THROW(domain_cast<uint8_t>(charMaxPlusOne), std::domain_error);
}