#include "B3L/MemberFunctionCalllback.h"
#include <gtest/gtest.h>

TEST(MemberFunctionCalllbackTests, UniqueBindings) {
    struct Foo {
        void foo() {
        }
    } foo0, foo1;

    auto callback0 = B3L_MAKE_UNIQUE_MEMBERFUNCTION_CALLBACK(&foo0, &Foo::foo);
    auto callback1 = B3L_MAKE_UNIQUE_MEMBERFUNCTION_CALLBACK(&foo1, &Foo::foo);

    EXPECT_NE(callback0, callback1);
}

TEST(MemberFunctionCalllbackTests, ThrowOnDuplicateBind) {
    struct Foo {
        void foo() {
        }
    } foo;

    auto callback0 = B3L::bindInstance(&foo, &Foo::foo);
    EXPECT_THROW(auto callback1 = B3L::bindInstance(&foo, &Foo::foo), std::exception);
}

TEST(MemberFunctionCalllbackTests, Invoke) {
    struct Foo {
        int foo() {
            return 123;
        }
    } foo;

    auto callback = B3L::bindInstance(&foo, &Foo::foo);
    EXPECT_EQ(callback(), foo.foo());
}