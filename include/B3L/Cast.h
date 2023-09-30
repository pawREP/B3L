#pragma once
#include "Define.h"
#include <bit>
#include <ios>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace B3L {

    template <typename To, typename From>
    [[nodiscard]] B3L_FORCEINLINE To rcast(From t) noexcept {
        return reinterpret_cast<To>(t);
    };

    template <typename To, typename From>
    [[nodiscard]] constexpr B3L_FORCEINLINE To scast(From t) noexcept {
        return static_cast<To>(t);
    };

    template <typename To, typename From>
    [[nodiscard]] constexpr B3L_FORCEINLINE To ccast(From t) noexcept {
        return const_cast<To>(t);
    };

    template <typename To, typename From>
    [[nodiscard]] B3L_FORCEINLINE To dcast(From t) {
        return dynamic_cast<To>(t);
    };

    template <typename To, typename From>
    [[nodiscard]] constexpr inline To domain_cast(From v) {
        if(!std::in_range<To>(v))
            throw std::domain_error("value can't be represented in target domain");

        return static_cast<To>(v);
    }

    template <std::unsigned_integral From>
    [[nodiscard]] constexpr inline auto sign_cast(From v) {
        return domain_cast<std::make_signed_t<From>>(v);
    }

    template <std::signed_integral From>
    [[nodiscard]] constexpr inline auto sign_cast(From v) {
        return domain_cast<std::make_unsigned_t<From>>(v);
    }

    template <typename Enum>
    [[nodiscard]] constexpr inline auto to_underlying(Enum en) noexcept {
#ifdef __cpp_lib_to_underlying
        return std::to_underlying(en);
#else
        return static_cast<std::underlying_type_t<Enum>>(en);
#endif
    };

    template <typename To, typename From>
    [[nodiscard]] constexpr inline To bit_cast(From from) noexcept(std::is_nothrow_constructible_v<To>) {

#ifdef __cpp_lib_bit_cast
        return std::bit_cast<To>(from);
#else
        static_assert(sizeof(To) == sizeof(From));
        static_assert(std::is_trivially_copyable_v<To>);
        static_assert(std::is_trivially_copyable_v<From>);

        static_assert(std::is_default_constructible_v<To>,
                      "Fallback impl of bit_cast requires is_default_constructible_v");

        To to;
        std::copy(&to, &from, sizeof(to));
#endif
    }

} // namespace B3L
