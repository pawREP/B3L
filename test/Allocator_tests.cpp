#include "B3L/Allocator.h"
#include <Windows.h>
#include <concepts>
#include <gtest/gtest.h>
#include <limits>
#include <vector>

using namespace B3L;

// Test whether allocated memory is within int32_t::max to base module load address
TEST(NearAllocatorTests, AllocationWithHint) {
    VirtualAllocAllocator<uint8_t, PAGE_READWRITE> allocator;

    auto hint = reinterpret_cast<uint8_t*>(GetModuleHandle(NULL));
    auto mem  = allocator.allocate(0x10, hint);

    auto dif = std::distance(mem, hint);
    EXPECT_LE(dif, (std::numeric_limits<int32_t>::max)());
}

// Test page protection
TEST(NearAllocatorTests, PageProtection) {
    DWORD old{};

    std::vector<std::byte, VirtualAllocAllocator<std::byte, PAGE_READWRITE>> mem{ 1 };
    EXPECT_TRUE(VirtualProtect(mem.data(), 1, PAGE_READWRITE, &old));
    EXPECT_EQ((DWORD)PAGE_READWRITE, old);

    std::vector<std::byte, VirtualAllocAllocator<std::byte, PAGE_EXECUTE_READWRITE>> memRWX{ 1 };
    EXPECT_TRUE(VirtualProtect(memRWX.data(), 1, PAGE_EXECUTE_READWRITE, &old));
    EXPECT_EQ((DWORD)PAGE_EXECUTE_READWRITE, old);
}
