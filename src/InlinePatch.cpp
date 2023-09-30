#ifdef B3L_HAVE_ASSEMBLERS
    #include "InlinePatch.h"
    #include "Allocator.h"
    #include "Assembler.h"
    #include "Disassembler.h"
    #include "Memory.h"
    #include <numeric>

using namespace B3L;

InlinePatch::InlinePatch(void* _address, const std::string& assembly) {
    uint8_t* const entrypoint = rcast<uint8_t*>(_address);
    enabled                   = std::make_unique<bool>(false);

    Assembler::Assembler<> assembler;
    // check enable/disabled
    assembler.push_back("push rax");
    assembler.push_back("mov rax, {}", Assembler::Address{ enabled.get() });
    assembler.push_back("movzx rax, byte ptr [rax]");
    assembler.push_back("cmp rax, 0x00");
    assembler.push_back("pop rax");
    assembler.push_back("je return");

    // new code
    assembler.push_back(assembly);

    // relocated code

    size_t entrypointSize{};
    auto entrypointInstructions = InlineDetour::disassembleEntrypoint(entrypoint, &entrypointSize);

    assembler.push_back("return:");
    for(const auto& insn : entrypointInstructions)
        assembler.push_back(insn.toString());

    // Jmp back.
    assembler.push_back("push rax");
    assembler.push_back("mov rax, {}", Assembler::Address(entrypoint + entrypointSize));
    assembler.push_back("xchg [rsp], rax");
    assembler.push_back("ret");

    size_t allocationSize = 0x1000; // One page
    {
        Allocator allocator{};
        auto mem = allocator.allocate(allocationSize);
        assert(mem);
        code.reset(mem);
    }

    assembler.assemble(code.get(), allocationSize);
    Memory::setPageProtection(code.get(), allocationSize, PAGE_EXECUTE_READ);

    detour = InlineDetour(entrypoint, code.get());
}

void B3L::InlinePatch::enable() {
    *enabled = true;
}

void B3L::InlinePatch::disable() {
    *enabled = false;
}

#endif
