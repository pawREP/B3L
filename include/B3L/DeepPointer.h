#pragma once
#include "Memory.h"
#include <Windows.h>
#include <initializer_list>
#include <optional>
#include <stdexcept>
#include <vector>

namespace B3L {

    template <typename T>
    class DeepPointer {
    public:
        using value_type   = T;
        using pointer_type = T*;

        DeepPointer() = default;
        explicit DeepPointer(nullptr_t) : DeepPointer() {
        }

        explicit DeepPointer(T* ptr) : DeepPointer(ptr, { 0 }) {
        }

        DeepPointer(void* base, std::initializer_list<int> offsets)
        : base(reinterpret_cast<uintptr_t>(base)), offsets(offsets) {
            if(!isValidState(this->base, this->offsets))
                throw std::invalid_argument("Invalid arguments");
        }

        DeepPointer(void* base, const std::vector<int>& offsets)
        : base(reinterpret_cast<uintptr_t>(base)), offsets(offsets) {
            if(!isValidState(this->base, this->offsets))
                throw std::invalid_argument("Invalid arguments");
        }

        DeepPointer(void* base, std::vector<int>&& offsets)
        : base(reinterpret_cast<uintptr_t>(base)), offsets(offsets) {
            if(!isValidState(this->base, this->offsets))
                throw std::invalid_argument("Invalid arguments");
        }

        DeepPointer(DeepPointer&&) noexcept                 = default;
        DeepPointer(const DeepPointer&) noexcept            = default;
        ~DeepPointer()                                      = default;
        DeepPointer& operator=(const DeepPointer&) noexcept = default;
        DeepPointer& operator=(DeepPointer&&) noexcept      = default;

        DeepPointer& operator=(nullptr_t) noexcept {
            DeepPointer other(nullptr);
            std::swap(*this, other);
            return *this;
        }

        DeepPointer& operator=(pointer_type ptr) {
            DeepPointer other(ptr);
            std::swap(*this, other);
            return *this;
        }

        [[nodiscard]] operator bool() const noexcept {
            return getAddress();
        }

        [[nodiscard]] std::optional<T> get() const noexcept {
            auto addr = getAddress();
            if(!addr)
                return std::nullopt;

            __try {
                return *addr;
            } __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
                return std::nullopt;
            }
        }

        bool set(value_type v) noexcept {
            auto addr = getAddress();
            if(!addr)
                return false;

            __try {
                *addr = v;
            } __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
                return false;
            }
            return true;
        }

        [[nodiscard]] value_type getOr(value_type def) const noexcept {
            auto result = get();
            if(result)
                return result.value();

            return def;
        }

        // Returns final pointer to held value, nullptr on failure.
        // This function should only be used to inspect the pointer, dereferencing it is generally not safe as the
        // pointed to memory is typically not under the control of the user and may become invalid at any moment.
        // Use get/set to access/modify the value instead.
        [[nodiscard]] pointer_type getAddress() const noexcept {
            if(!offsets.size())
                return nullptr;

            __try {
                uintptr_t addr = base + offsets[0];
                for(size_t i = 1; i < offsets.size(); ++i) {
                    if(!Memory::isInApplicationAddressRange(addr))
                        return nullptr;
                    addr = *reinterpret_cast<uintptr_t*>(addr);
                    addr += offsets[i];
                }
                if(!Memory::isInApplicationAddressRange(addr))
                    return nullptr;
                return reinterpret_cast<pointer_type>(addr);
            } __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
                return nullptr;
            }
        }

        // Checks whether the base address and offsets could form a valid DeepPointer
        [[nodiscard]] static bool isValidState(uintptr_t base, const std::vector<int>& offsets) {
            return base && Memory::isInApplicationAddressRange(base) && offsets.size();
        }

    private:
        uintptr_t base{};
        std::vector<int> offsets{};
    };

    template <typename T0>
    [[nodiscard]] bool operator==(const DeepPointer<T0>& lhs, const DeepPointer<T0>& rhs) noexcept {
        return lhs.getAddress() == rhs.getAddress();
    }

    template <typename T0>
    [[nodiscard]] bool operator==(const DeepPointer<T0>& lhs, T0* ptr) noexcept {
        return lhs.getAddress() == ptr;
    }

    template <typename T0>
    [[nodiscard]] bool operator==(T0* ptr, const DeepPointer<T0>& rhs) noexcept {
        return rhs.getAddress() == ptr;
    }

    template <typename T0>
    [[nodiscard]] bool operator==(const DeepPointer<T0>& lhs, nullptr_t) noexcept {
        return !lhs.getAddress();
    }

    template <typename T0>
    [[nodiscard]] bool operator==(nullptr_t, const DeepPointer<T0>& rhs) noexcept {
        return !rhs.getAddress();
    }

} // namespace B3L
