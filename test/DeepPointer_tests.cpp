#include "B3L/DeepPointer.h"
#include <gtest/gtest.h>
#include <limits>
#include <memory>
#include <vector>

using namespace B3L;

class DeepPointerTests : public ::testing::Test {
protected:
    DeepPointerTests() = default;

    void SetUp() override {
        bar = {};
        foo = { &bar };
        ptrValid = std::unique_ptr<DeepPointer<int>>(new DeepPointer<int>(&foo, { offsetof(Foo, b), offsetof(Bar, val) }));
        ptrInvalid = std::unique_ptr<DeepPointer<int>>(new DeepPointer<int>(&foo, { 0, offsetof(Bar, val) }));
    }

    std::unique_ptr<DeepPointer<int>> ptrValid;
    std::unique_ptr<DeepPointer<int>> ptrInvalid;

    static inline constexpr int deepPtrSollValue = 69;

private:
    struct Bar {
        int _0{};
        int _1{};
        int _2{};
        int val = deepPtrSollValue;
    };

    struct Foo {
        Foo() = default;
        Foo(Bar* bar) : b(bar){};
        int _0{};
        int _1{};
        Bar* b = nullptr;
    };

    Bar bar{};
    Foo foo{};
};


TEST_F(DeepPointerTests, InvalidConstruction) {
    // Empty offset list
    int i{};
    EXPECT_THROW(DeepPointer<int> invalidPtr(&i, {}), std::invalid_argument);
    // Invalid base
    EXPECT_THROW(DeepPointer<int> invalidPtr2(nullptr, { 1, 2, 3 }), std::invalid_argument);
}

TEST_F(DeepPointerTests, get) {
    // valid
    EXPECT_EQ(ptrValid->get().value(), deepPtrSollValue);

    // invalid
    EXPECT_FALSE(ptrInvalid->get().has_value());
}

TEST_F(DeepPointerTests, setValid) {
    int newVal = deepPtrSollValue + 1;
    // valid
    EXPECT_NE(ptrValid->get().value(), newVal);
    EXPECT_TRUE(ptrValid->set(newVal));
    EXPECT_EQ(ptrValid->get().value(), newVal);

    // invalid
    EXPECT_FALSE(ptrInvalid->set(newVal));
}

TEST_F(DeepPointerTests, getOr) {
    int defaultValue = deepPtrSollValue + 1;

    // valid
    EXPECT_EQ(ptrValid->getOr(defaultValue), deepPtrSollValue);
    // invalid
    EXPECT_EQ(ptrInvalid->getOr(defaultValue), defaultValue);
}

TEST_F(DeepPointerTests, Compare) {
    static_assert(std::equality_comparable<DeepPointer<int>>);

    DeepPointer<int> a;
    EXPECT_TRUE(a == nullptr);
    EXPECT_TRUE(a == reinterpret_cast<int*>(0));
}

TEST_F(DeepPointerTests, Construction) {
    static_assert(std::is_default_constructible_v<DeepPointer<int>>);
    static_assert(std::is_nothrow_move_constructible_v<DeepPointer<int>>);

    static_assert(std::is_constructible_v<DeepPointer<int>, void*, std::initializer_list<int>>);
    static_assert(std::is_constructible_v<DeepPointer<int>, nullptr_t>);
    static_assert(std::is_constructible_v<DeepPointer<int>, int*>);

    static_assert(std::is_copy_assignable_v<DeepPointer<int>>);
    static_assert(std::is_nothrow_move_assignable_v<DeepPointer<int>>);

    static_assert(std::is_assignable_v<DeepPointer<int>, nullptr_t>);
    static_assert(std::is_assignable_v<DeepPointer<int>, int*>);
}

TEST_F(DeepPointerTests, ConvertableToBool) {
    static_assert(std::is_convertible_v<DeepPointer<int>, bool>);

    DeepPointer<int> a;
    ASSERT_FALSE(a);

    int i = 123;
    DeepPointer<int> b(&i);
    ASSERT_TRUE(b);
}
