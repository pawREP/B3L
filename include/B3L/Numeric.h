#pragma once
#include <concepts>
#include <limits>
#include <stdexcept>

namespace B3L {
    namespace Numeric {

        // Negates a signed integral value without overflow for min value.
        template <std::signed_integral Int>
        [[nodiscard]] Int negate(Int v) {
            if(v == (std::numeric_limits<Int>::min)())
                ++v;

            return -v;
        }

    } // namespace Numeric
} // namespace B3L
