#include "B3L/InlineCallbackHook.h"
#include <gtest/gtest.h>

using namespace B3L;

TEST(InlinePatchTests, BasicOperation) {
    int value = 123;

    std::string assembly{ "inc rcx" };

    int (*foo)(int) = [](int a) { return a; };

    {
        InlinePatch inlinePatch(foo, assembly);

        EXPECT_EQ(foo(value), value); // InlinePatch is disabled on construction
        inlinePatch.enable();
        EXPECT_EQ(foo(value), value + 1);
        inlinePatch.disable();
        EXPECT_EQ(foo(value), value);
        inlinePatch.enable();
    } // ~InlinePatch()

    EXPECT_EQ(foo(value), value); // Making sure dtor doesn't corrupted hooked function
}

TEST(InlinePatchTests, Move) {
    int value = 123;

    std::string assembly{ "inc rcx" };

    int (*foo)(int) = [](int a) { return a; };

    {
        InlinePatch inlinePatch(foo, assembly);
        InlinePatch inlinePatch2(std::move(inlinePatch));

        EXPECT_EQ(foo(value), value); // InlinePatch is disabled on construction
        inlinePatch2.enable();
        EXPECT_EQ(foo(value), value + 1);
        inlinePatch2.disable();
        EXPECT_EQ(foo(value), value);
        inlinePatch2.enable();
    } // ~InlinePatch()

    EXPECT_EQ(foo(value), value); // Making sure dtor doesn't corrupted hooked function
}

class InlineHookTests : public ::testing::Test {
protected:
    InlineHookTests() = default;

    void SetUp() override {
        flag  = false;
        value = 0;
    }

    static void setValue(int* val) {
        value = *val;
    }
    static int getValue() {
        return value;
    }

    static void setFlagTrue(void*) {
        flag = true;
    }

    static bool isFlagSet() {
        return flag;
    }

    static inline bool flag;
    static inline int value;
};

TEST_F(InlineHookTests, InlineHookBasic) {
    int (*add)(int, int) = [](int a, int b) { return a + b; };

    EXPECT_EQ(flag, false);
    {
        InlineCallback inlineHook(add, &setFlagTrue);
        EXPECT_EQ(flag, false);
        inlineHook.enable();
        EXPECT_EQ(add(1, 2), 3);
        EXPECT_EQ(flag, true);
        inlineHook.disable();
        flag = false;
        EXPECT_EQ(add(1, 2), 3);
        EXPECT_EQ(flag, false);
        inlineHook.enable();
    } //~InlineCallback()
    EXPECT_EQ(add(1, 2), 3);
    EXPECT_EQ(flag, false);
}

TEST_F(InlineHookTests, InlineHookWithUserPtr) {
    int (*square)(int) = [](int a) { return a * a; };

    int newValue = 2;

    EXPECT_EQ(value, 0);
    InlineCallback inlineHook(square, &setValue, &newValue);
    EXPECT_EQ(value, 0);
    inlineHook.enable();
    EXPECT_EQ(square(2), 4);
    EXPECT_EQ(value, 2);
}
