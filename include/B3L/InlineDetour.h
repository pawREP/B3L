#pragma once
#ifdef B3L_HAVE_ASSEMBLERS
    #include "Allocator.h"
    #include "Define.h"
    #include "Instruction.h"
    #include <cstdint>

namespace B3L {

    class InlineDetour {
        B3L_MAKE_NONCOPYABLE(InlineDetour);

    public:
        InlineDetour() = default;
        InlineDetour(uint8_t* entrypoint, const uint8_t* target);
        InlineDetour(InlineDetour&& other) noexcept;
        InlineDetour& operator=(InlineDetour&& other) noexcept;
        ~InlineDetour();

        static std::vector<Instruction> disassembleEntrypoint(uint8_t* entrypoint, size_t* size = nullptr);

    protected:
        std::vector<Instruction> entrypointInstructions;
        size_t entrypointSize{};

    private:
        void detourEntrypoint(const uint8_t* target);
        void restoreEntrypoint();

        static const int minEntrypointSize = 5; // Smallest possible entrypoint size. (size of relative jmp instruction)

        uint8_t* entrypoint = nullptr;

        using Allocator = VirtualAllocAllocator<uint8_t, PAGE_EXECUTE_READWRITE>;
        std::unique_ptr<Allocator::value_type, deleter_trait_t<Allocator>> trampoline = nullptr;
    };

} // namespace B3L

#endif
