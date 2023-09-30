#ifdef B3L_HAVE_ASSEMBLERS
    #include "B3L/Define.h"
    #include "B3L/InlineCallbackHook.h"
    #include <gtest/gtest.h>

using namespace B3L;

namespace {
    B3L_NEVERINLINE int foo(int a, int b = 0) {
        return a + b;
    };
} // namespace

TEST(InlinePatchTests, BasicOperation) {
    volatile int value = 123;

    std::string assembly{ "inc rcx" };

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
    volatile int value = 123;

    std::string assembly{ "inc rcx" };

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

namespace {
    B3L_NEVERINLINE int add(int a, int b) {
        return a + b;
    };
} // namespace

TEST_F(InlineHookTests, InlineHookBasic) {

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

namespace {
    B3L_NEVERINLINE int square(int a) {
        return a * a;
    };
} // namespace

TEST_F(InlineHookTests, InlineHookWithUserPtr) {

    int newValue = 2;

    EXPECT_EQ(value, 0);
    InlineCallback inlineHook(square, &setValue, &newValue);
    EXPECT_EQ(value, 0);
    inlineHook.enable();
    EXPECT_EQ(square(2), 4);
    EXPECT_EQ(value, 2);
}

#endif
