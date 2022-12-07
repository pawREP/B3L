#pragma once
#include "Cast.h"
#include "Exception.h"
#include <Windows.h>
#include <cassert>
#include <cstddef>

namespace B3L {
    namespace Memory {

        // Returns whether an address is between the min and max application address of the system.
        [[nodiscard]] bool isInApplicationAddressRange(void* addr);
        [[nodiscard]] bool isInApplicationAddressRange(uintptr_t addr);

        // Changes page protection and returns old protection. Throws Win32Exception on failure.
        int setPageProtection(void* addr, size_t size, int protection);

        // Writes data to write protected memory. Throws Win32Expection on error.
        template <std::forward_iterator FwdIter>
        void writeProtectedMemory(void* addr, FwdIter begin, FwdIter end) {
            writeProtectedMemory(addr, begin, std::distance(begin, end));
        }

        template <std::input_iterator InIter>
        void writeProtectedMemory(void* addr, InIter begin, size_t size) {
            static_assert(std::is_same_v<typename std::iterator_traits<InIter>::value_type, uint8_t>);

            auto oldProtextion = Memory::setPageProtection(addr, size, PAGE_EXECUTE_READWRITE);
            std::copy_n(begin, size, scast<uint8_t*>(addr));
            Memory::setPageProtection(addr, size, oldProtextion);
        }

        // Writes value to write protected memory. Throws Win32Expection on error.
        template <typename T>
        void writeProtectedMemory(void* addr, const T& v) {
            static_assert(std::is_trivially_copyable_v<T>);
            writeProtectedMemory(addr, reinterpret_cast<const std::uint8_t*>(&v), sizeof(v));
        }

        // Type-safe wrapper around writeProtectedMemory intended for swapping function pointers in vtables.
        template <typename Ret, typename... Args>
        void swapFunctionPointer(Ret (**addr)(Args...), Ret (*proc)(Args...), Ret (**original)(Args...) = nullptr) {
            if(original)
                *original = *addr;
            writeProtectedMemory(addr, proc);
        }

    } // namespace Memory
} // namespace B3L
