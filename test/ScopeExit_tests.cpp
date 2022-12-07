#include "B3L/ScopeExit.h"
#include <gtest/gtest.h>

using namespace B3L;

class ScopeExitTests : public ::testing::Test {
protected:
    ScopeExitTests() = default;

    void SetUp() override {
        flag = false;
    }

    static void setFlagTrue() {
        flag = true;
    }

    static inline bool flag = false;
};

TEST_F(ScopeExitTests, Lambda) {
    {
        ScopeExit _{ []() { flag = true; } };
        EXPECT_FALSE(flag);
    }
    EXPECT_TRUE(flag);
}

TEST_F(ScopeExitTests, FunctionPointer) {
    {
        ScopeExit _{ &setFlagTrue };
        EXPECT_FALSE(flag);
    }
    EXPECT_TRUE(flag);
}

TEST_F(ScopeExitTests, StdFunction) {
    {
        auto fn = std::function([]() { flag = true; });
        ScopeExit _{ std::move(fn) };
        EXPECT_FALSE(flag);
    }
    EXPECT_TRUE(flag);
}

TEST(ScopeFailureTests, OnlyExcuteWhileExceptionInFlight) {
    bool flag = false;

    [&flag]() {                                       //
        ScopeFailure _{ [&flag]() { flag = true; } }; //
    }();

    EXPECT_FALSE(flag);

    try {
        [&flag]() {                                       //
            ScopeFailure _{ [&flag]() { flag = true; } }; //
            throw std::exception("Oh no!");
        }();
    } catch(...) {
    }

    EXPECT_TRUE(flag);
}

TEST(ScopeSuccessTests, OnlyExcuteWhileNoExceptionInFlight) {
    bool flag = false;

    try {
        [&flag]() {                                       //
            ScopeSuccess _{ [&flag]() { flag = true; } }; //
            throw std::exception("Oh no!");
        }();
    } catch(...) {
    }

    EXPECT_FALSE(flag);

    [&flag]() {                                       //
        ScopeSuccess _{ [&flag]() { flag = true; } }; //
    }();

    EXPECT_TRUE(flag);
}