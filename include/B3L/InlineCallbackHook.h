#pragma once
#ifdef B3L_HAVE_ASSEMBLERS
    #include "Assembler.h"
    #include "Hook.h"
    #include "InlinePatch.h"

namespace B3L {

    namespace detail {
        inline constexpr const char* pushAll = //
        "push rax;"
        "push rbx;"
        "push rcx;"
        "push rdx;"
        "push rsi;"
        "push rdi;"
        "push r8;"
        "push r9;"
        "push r10;"
        "push r11;"
        "push r12;"
        "push r13;"
        "push r14;"
        "push r15;"
        "lea rsp, [rsp - 0x100];"
        "movupd xmmword ptr [rsp + 0x00], xmm0;"
        "movupd xmmword ptr [rsp + 0x10], xmm1;"
        "movupd xmmword ptr [rsp + 0x20], xmm2;"
        "movupd xmmword ptr [rsp + 0x30], xmm3;"
        "movupd xmmword ptr [rsp + 0x40], xmm4;"
        "movupd xmmword ptr [rsp + 0x50], xmm5;"
        "movupd xmmword ptr [rsp + 0x60], xmm6;"
        "movupd xmmword ptr [rsp + 0x70], xmm7;"
        "movupd xmmword ptr [rsp + 0x80], xmm8;"
        "movupd xmmword ptr [rsp + 0x90], xmm9;"
        "movupd xmmword ptr [rsp + 0xA0], xmm10;"
        "movupd xmmword ptr [rsp + 0xB0], xmm11;"
        "movupd xmmword ptr [rsp + 0xC0], xmm12;"
        "movupd xmmword ptr [rsp + 0xD0], xmm13;"
        "movupd xmmword ptr [rsp + 0xE0], xmm14;"
        "movupd xmmword ptr [rsp + 0xF0], xmm15;";
        //"pushf;";

        inline constexpr const char* popAll = //"popf;"
        "movupd xmm15, xmmword ptr [rsp + 0xF0];"
        "movupd xmm14, xmmword ptr [rsp + 0xE0];"
        "movupd xmm13, xmmword ptr [rsp + 0xD0];"
        "movupd xmm12, xmmword ptr [rsp + 0xC0];"
        "movupd xmm14, xmmword ptr [rsp + 0xB0];"
        "movupd xmm10, xmmword ptr [rsp + 0xA0];"
        "movupd xmm9, xmmword ptr [rsp + 0x90];"
        "movupd xmm8, xmmword ptr [rsp + 0x80];"
        "movupd xmm7, xmmword ptr [rsp + 0x70];"
        "movupd xmm6, xmmword ptr [rsp + 0x60];"
        "movupd xmm5, xmmword ptr [rsp + 0x50];"
        "movupd xmm4, xmmword ptr [rsp + 0x40];"
        "movupd xmm3, xmmword ptr [rsp + 0x30];"
        "movupd xmm2, xmmword ptr [rsp + 0x20];"
        "movupd xmm1, xmmword ptr [rsp + 0x10];"
        "movupd xmm0, xmmword ptr [rsp + 0x00];"
        "lea rsp, [rsp + 0x100];"
        "pop r15;"
        "pop r14;"
        "pop r13;"
        "pop r12;"
        "pop r11;"
        "pop r10;"
        "pop r9;"
        "pop r8;"
        "pop rdi;"
        "pop rsi;"
        "pop rdx;"
        "pop rcx;"
        "pop rbx;"
        "pop rax;";
    } // namespace detail

    // Detour code at given target address and transfers control to a callback function.
    class InlineCallback {
        B3L_MAKE_NONCOPYABLE(InlineCallback);

    public:
        template <typename T>
        InlineCallback(void* target, void (*callback)(T* userPtr), T* userPtr = nullptr);
        InlineCallback(InlineCallback&&) noexcept            = default;
        InlineCallback& operator=(InlineCallback&&) noexcept = default;
        ~InlineCallback()                                    = default;

        void enable();
        void disable();

    private:
        std::unique_ptr<InlinePatch> inlinePatch = nullptr;
    };

    template <typename T>
    inline B3L::InlineCallback::InlineCallback(void* target, void (*callback)(T*), T* userPtr) {
        Assembler::Assembler<> assembler;

        assembler.push_back(detail::pushAll);

        assembler.push_back("mov rcx, {}", Assembler::Address{ userPtr });
        assembler.push_back("mov rax, {}", Assembler::Address{ callback });
        assembler.push_back("call rax");

        assembler.push_back(detail::popAll);

        inlinePatch = std::make_unique<InlinePatch>(target, assembler.assembly());
    }

} // namespace B3L

#endif
