#include "B3L/PrologHook.h"
#include "B3L/Define.h"
#include <gtest/gtest.h>
#include <limits>

using namespace B3L;

B3L_NEVERINLINE double originalBigFunction(float a, int b, int c, size_t d, double e) {
    return a * b + c - d + e;
}

B3L_NEVERINLINE double newBigFunction(float a, int b, int c, size_t d, double e) {
    return a * (b + c + d + e);
}

TEST(PrologHookTest, Test0) {
    auto oldr = originalBigFunction(2, 3, 4, 5, 6);
    auto newr = newBigFunction(2, 3, 4, 5, 6);

    PrologHook prologHook(&originalBigFunction, &newBigFunction);
    EXPECT_EQ(originalBigFunction(2, 3, 4, 5, 6), newr);
}