#pragma once
#include "Allocator.h"
#include "Cast.h"
#include <Windows.h>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>

namespace B3L {

    struct NullThunk {};

    template <typename ThunkData>
    class Thunk {
        static_assert(std::is_copy_constructible_v<ThunkData>);

    public:
        virtual ~Thunk() = default;

        void* entrypoint() {
            return thunk.get();
        }

    protected:
        Thunk() = default;

        template <typename... Args>
        Thunk(Args... args) {
            static_assert(std::is_constructible_v<ThunkData, Args...>, "Invalid arguments");

            thunk = allocateThunk();
            new(thunk.get()) ThunkData(std::forward<Args>(args)...);
            markThunkReadExecute(thunk.get());
        }

        Thunk(const Thunk& other) {
            *this = other;
        }

        Thunk& operator=(const Thunk& other) {
            thunk = allocateThunk();
            new(thunk.get()) ThunkData(*(other.thunk.get()));
            markThunkReadExecute(thunk.get());
            return *this;
        }

        Thunk(Thunk&& other) noexcept            = default;
        Thunk& operator=(Thunk&& other) noexcept = default;

    private:
        using Deleter = typename deleter_trait<VirtualAllocAllocator<ThunkData, PAGE_EXECUTE_READWRITE>>::type;

        static std::unique_ptr<ThunkData, Deleter> allocateThunk() {
            VirtualAllocAllocator<ThunkData, PAGE_EXECUTE_READWRITE> allocator;
            auto mem = allocator.allocate(1);
            return std::unique_ptr<ThunkData, Deleter>(mem);
        }

        static void markThunkReadExecute(ThunkData* thunk) {
            DWORD old{};
            assert(VirtualProtect(thunk, sizeof(ThunkData), PAGE_EXECUTE_READ, &old));
        }

        std::unique_ptr<ThunkData, Deleter> thunk = nullptr;
    };

    Thunk()->Thunk<NullThunk>;

    namespace detail {

#pragma pack(push, 1)
        class MFThunkDataX64 {
        public:
            MFThunkDataX64(void* instance, void* target)
            : instance(reinterpret_cast<uint64_t>(instance)), target(reinterpret_cast<uint64_t>(target)) {
            }

        private:
            const uint8_t _0[0x0B] = { 0x4D, 0x89, 0xC1, // mov r9, r8
                                       0x49, 0x89, 0xD0, // mov r8, rdx
                                       0x48, 0x89, 0xCA, // mov rdx, rcx
                                       0x48, 0xB9 };     // mov rcx, this_
            const uintptr_t instance{};
            const uint8_t _1[0x03] = { 0x50,         // push rax
                                       0x48, 0xB8 }; // mov rax, target
            const uintptr_t target{};
            const uint8_t _2[0x05] = { 0x48, 0x87, 0x04, 0x24, // xchg [rsp], rax
                                       0xC3 };                 // ret
        };
#pragma pack(pop)

#if _WIN64 // TODO: x86 thunking support.
        using MFThunkData = MFThunkDataX64;
#endif

        template <typename First, typename... Rest>
        consteval bool isThunkCompatibleParamPack() {
            if constexpr(sizeof...(Rest) == 0) {
                return (std::is_integral_v<First> || std::is_pointer_v<First>);
            } else {
                return (std::is_integral_v<First> || std::is_pointer_v<First>)&&isThunkCompatibleParamPack<Rest...>();
            }
        }

    } // namespace detail

    template <typename Ret, typename... Args>
    class MemberFunctionCallThunk : public Thunk<detail::MFThunkData> {
    public:
        using pointer = Ret (*)(Args...);

        MemberFunctionCallThunk() = default;

        template <typename Class>
        MemberFunctionCallThunk(Class* instance, Ret (Class::*mf)(Args...))
        : Thunk(instance, std::bit_cast<void*>(mf)) {
#if !_WIN64
            throw std::runtime_error("Thunking only supports x64");
#endif
            static_assert(sizeof...(Args) <= 3, "Thunk only supports functions with up to 3 arguments");
            if constexpr(sizeof...(Args))
                static_assert(detail::isThunkCompatibleParamPack<Args...>(),
                              "Function arguments have to be integral or pointers.");
        }

        Ret operator()(Args... args) {
            return reinterpret_cast<pointer>(entrypoint())(std::forward<Args>(args)...);
        }
    };

    MemberFunctionCallThunk()->MemberFunctionCallThunk<void>;

} // namespace B3L
