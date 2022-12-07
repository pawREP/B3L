#pragma once
#include "B3L/Exception.h"
#include <Windows.h>
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <new>

namespace B3L {

    template <typename Allocator>
    struct deleter_trait;

    template <typename T>
    struct deleter_trait<typename std::allocator<T>> {
        using type = std::default_delete<T>;
    };

    template <typename T>
    using deleter_trait_t = typename deleter_trait<T>::type;

    struct VirtualFreeDeleter {
        void operator()(void* p) const {
            VirtualFree(p, 0, MEM_RELEASE);
        }
    };

    // VirtualAlloc based, STL compatible allocator.
    // This allocator is guaranteed to return pages that aren't shared with any other allocation regardless of allocation size.
    // Beyond the standard requirements this allocator provides a hint interface that allows allocation near a specified address.
    template <typename T, int PageProtection = PAGE_READWRITE>
    struct VirtualAllocAllocator {
        using value_type = T;

        VirtualAllocAllocator() = default;

        template <typename U, int P>
        constexpr VirtualAllocAllocator(const VirtualAllocAllocator<U, P>&) noexcept {
        }

        [[nodiscard]] T* allocate(std::size_t n, void* hint) const {
            // TODO: Currently spits out chunks of allocation granularity size (16 pages). We could reduce that to page size.

            if(n > (std::numeric_limits<std::size_t>::max)() / sizeof(T))
                throw std::bad_array_new_length();

            const auto allocationSize = n * sizeof(T);

            static SYSTEM_INFO info = []() {
                SYSTEM_INFO _info;
                GetSystemInfo(&_info);
                return _info;
            }();

            auto target  = reinterpret_cast<uint8_t*>(hint);
            void* mem    = nullptr;
            int64_t disp = 0;

            while(!mem) {
                mem = VirtualAlloc(target + disp, allocationSize, MEM_RESERVE | MEM_COMMIT, PageProtection);

                disp = -disp;
                if(disp >= 0)
                    disp += info.dwAllocationGranularity;
            }

            return static_cast<T*>(mem);
        }

        [[nodiscard]] T* allocate(std::size_t n) const {
            return allocate(n, nullptr);
        }

        void deallocate(T* p, [[maybe_unused]] std::size_t n) const {
            if(!VirtualFree(p, 0, MEM_RELEASE))
                throw Win32Exception("VirtualFree");
        }

        template <class U>
        struct rebind {
            using other = VirtualAllocAllocator<U, PageProtection>;
        };
    };

    template <class T, class U, int V0, int V1>
    bool operator==(const VirtualAllocAllocator<T, V0>&, const VirtualAllocAllocator<U, V1>&) {
        return true;
    }

    template <class T, class U, int V0, int V1>
    bool operator!=(const VirtualAllocAllocator<T, V0>&, const VirtualAllocAllocator<U, V1>&) {
        return false;
    }

    template <typename T, int P>
    struct deleter_trait<VirtualAllocAllocator<T, P>> {
        using type = VirtualFreeDeleter;
    };

} // namespace B3L
