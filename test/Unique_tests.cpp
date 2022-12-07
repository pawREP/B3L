#include "B3L/Unique.h"
#include <gtest/gtest.h>

namespace {

    struct Foo : B3L::Unique<Foo> {};

} // namespace

TEST(UniqueTests, CanOnlyCreateOne) {
    Foo foo;
    EXPECT_THROW(Foo(), std::exception);
}

TEST(UniqueTests, DtorFreesUniqueLock) {
    { Foo foo; }
    Foo foo;
}