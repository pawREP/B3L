#include "B3L/Define.h"
#include "B3L/PrologHook.h"
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
    EXPECT_EQ(prologHook.invokeOriginal<double (*)(float, int, int, size_t, double)>(2, 3, 4, 5, 6), oldr);
}

DWORD MyGetCurrentProcessId() {
    return 0;
}

TEST(PrologHookTest, Test1) {
    auto realPid = GetCurrentProcessId();
    EXPECT_NE(realPid, 0);

    PrologHook prologHook(&GetCurrentProcessId, &MyGetCurrentProcessId);

    auto fakePid = GetCurrentProcessId();
    EXPECT_EQ(fakePid, 0);

    auto realPid2 = prologHook.invokeOriginal<DWORD (*)()>();
    EXPECT_EQ(realPid, realPid2);
}