#include "B3L/Thunk.h"
#include <gtest/gtest.h>
#include <limits>

using namespace B3L;

namespace {

    struct Foo {
        int foo0(void) {
            return 0;
        }
        int foo1(int a) {
            return a;
        }
        int foo2(int a, int b) {
            return a + b;
        }
        int foo3(int a, int b, int c) {
            return a + b + c;
        }
    };

} // namespace

TEST(MemberFunctionCallThunkTests, Invoke) {
    Foo foo;

    MemberFunctionCallThunk thunk0(&foo, &Foo::foo0);
    MemberFunctionCallThunk thunk1(&foo, &Foo::foo1);
    MemberFunctionCallThunk thunk2(&foo, &Foo::foo2);
    MemberFunctionCallThunk thunk3(&foo, &Foo::foo3);

    EXPECT_EQ(thunk0(), 0);
    EXPECT_EQ(thunk1(1), 1);
    EXPECT_EQ(thunk2(1, 2), 3);
    EXPECT_EQ(thunk3(1, 2, 3), 6);
}

TEST(MemberFunctionCallThunkTests, DefaultConstruct) {
    MemberFunctionCallThunk a;
    EXPECT_EQ(a.entrypoint(), nullptr);
}

TEST(MemberFunctionCallThunkTests, SpecialFunction) {
    using thunk_t = MemberFunctionCallThunk<void>;

    static_assert(std::is_constructible_v<thunk_t>);
    static_assert(std::is_constructible_v<thunk_t, Foo*, void (Foo::*)()>);
    static_assert(std::is_copy_constructible_v<thunk_t>);
    static_assert(std::is_nothrow_move_constructible_v<thunk_t>);

    static_assert(std::is_copy_assignable_v<thunk_t>);
    static_assert(std::is_nothrow_move_assignable_v<thunk_t>);
}