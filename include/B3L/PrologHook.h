#pragma once
#include "Cast.h"
#include "Define.h"
#include "InlineDetour.h"

namespace B3L {

    class PrologHook : InlineDetour {
        B3L_MAKE_NONCOPYABLE(PrologHook);

    public:
        template <typename Ret, typename... Args>
        PrologHook(Ret (*target)(Args...),  Ret (*hook)(Args...))
        : InlineDetour(rcast<uint8_t*>(target), rcast<uint8_t*>(hook)){};
    };

} // namespace B3L
