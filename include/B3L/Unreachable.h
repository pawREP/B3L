#pragma once
#include "Define.h"
#include <utility>

[[noreturn]] void B3L_FORCEINLINE unreachable() {
#ifdef __cpp_lib_unreachable
    std::unreachable();
#else
    #if _MSC_VER
    __assume(false);
    #else
    __builtin_unreachable();
    #endif
#endif
}