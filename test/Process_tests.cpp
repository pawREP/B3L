#include "B3L/Process.h"
#include <Windows.h>
#include <gtest/gtest.h>

using namespace B3L;

namespace {
    intptr_t ptrToInt(void* ptrValid) {
        return reinterpret_cast<intptr_t>(ptrValid);
    }
} // namespace

TEST(ProcessTests, GetBaseModuleBase) {
    EXPECT_NE(ptrToInt(getModuleBaseAddress()), 0);
}

TEST(ProcessTests, GetNamedModuleBase) {
    EXPECT_NE(ptrToInt(getModuleBaseAddress("KERNEL32")), 0);
    EXPECT_NE(ptrToInt(getModuleBaseAddress("KERNEL32.dll")), 0);
}

TEST(ProcessTests, GetInvalidModuleBaseThrow) {
    EXPECT_EQ(getModuleBaseAddress("NonExistingModule"), nullptr);
}

TEST(ProcessTests, GetExecutableFileName) {
    EXPECT_EQ(getExecutableFileName(), "unit_tests.exe");
}