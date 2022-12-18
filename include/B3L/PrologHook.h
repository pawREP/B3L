#pragma once
#include "Cast.h"
#include "Define.h"
#include "InlineDetour.h"

namespace B3L {

    class PrologHook : InlineDetour {
        B3L_MAKE_NONCOPYABLE(PrologHook);

    public:
        template <typename Ret, typename... Args>
        PrologHook(Ret (*target)(Args...), Ret (*hook)(Args...));

        template <typename Sig>
        auto originalTarget() const {
            static_assert(std::is_pointer_v<Sig>);
            static_assert(std::is_function_v<std::remove_pointer_t<Sig>>);
            return reinterpret_cast<Sig>(originalEntryPointThunk.get());
        }

        template <typename Sig, typename... Args>
        auto invokeOriginal(Args&&... args) const {
            return originalTarget<Sig>()(std::forward<Args>(args)...);
        }

    private:
        using Allocator = VirtualAllocAllocator<uint8_t, PAGE_EXECUTE_READWRITE>;

        void buildoriginalEntryPointThunk();

        std::unique_ptr<Allocator::value_type, deleter_trait_t<Allocator>> originalEntryPointThunk = nullptr;
    };

    template <typename Ret, typename... Args>
    inline PrologHook::PrologHook(Ret (*target)(Args...), Ret (*hook)(Args...))
    : InlineDetour(rcast<uint8_t*>(target), rcast<uint8_t*>(hook)) {
        buildoriginalEntryPointThunk();
      };

} // namespace B3L
