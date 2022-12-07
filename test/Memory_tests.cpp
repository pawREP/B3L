#include "B3L/Memory.h"
#include <Windows.h>
#include <gtest/gtest.h>

using namespace B3L;

TEST(MemoryTests, WriteProtectedMemory) {
    auto mem = VirtualAlloc(NULL, sizeof(intptr_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    assert(mem);
    *reinterpret_cast<intptr_t*>(mem) = 0;

    DWORD old;
    assert(VirtualProtect(mem, sizeof(intptr_t), PAGE_READONLY, &old));

    intptr_t deepPtrSollValue = 123;
    Memory::writeProtectedMemory(mem, deepPtrSollValue);

    // Check if protection is unchanged
    assert(VirtualProtect(mem, sizeof(intptr_t), PAGE_READONLY, &old));
    EXPECT_EQ(old, PAGE_READONLY);

    EXPECT_EQ(*reinterpret_cast<intptr_t*>(mem), deepPtrSollValue);
}
