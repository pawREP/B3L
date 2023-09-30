#pragma once
#ifdef B3L_HAVE_ASSEMBLERS
    #include "Allocator.h"
    #include "Define.h"
    #include "Disassembler.h"
    #include "InlineDetour.h"
    #include <memory>
    #include <vector>

namespace B3L {

    class InlinePatch {
        B3L_MAKE_NONCOPYABLE(InlinePatch);

    public:
        InlinePatch(void* address, const std::string& assembly);
        InlinePatch(InlinePatch&&) noexcept            = default;
        InlinePatch& operator=(InlinePatch&&) noexcept = default;
        ~InlinePatch()                                 = default;

        void enable();
        void disable();

    private:
        using Allocator = VirtualAllocAllocator<uint8_t, PAGE_EXECUTE_READWRITE>;

        InlineDetour detour;
        std::unique_ptr<bool> enabled = nullptr; // Referenced in generated code, can't be on the stack!
        std::unique_ptr<Allocator::value_type, deleter_trait_t<Allocator>> code;
    };

} // namespace B3L

#endif
