#ifdef B3L_HAVE_ASSEMBLERS
    #include "PrologHook.h"
    #include "Assembler.h"

void B3L::PrologHook::buildoriginalEntryPointThunk() {

    Assembler::Assembler<> assembler;

    auto returnTarget = entrypointInstructions.back().address + entrypointInstructions.back().size;
    if(entrypointInstructions.size() == 1) {
        const auto& insn = entrypointInstructions.front();
        if(insn.id == X86_INS_JMP) {
            if(insn.operands[0].type == X86_OP_MEM) {
                auto cmem    = *reinterpret_cast<const int*>(&insn.bytes[2]);
                returnTarget = *reinterpret_cast<uint64_t*>(insn.address + insn.size + cmem);
            } else if(insn.operands[0].type == X86_OP_IMM) {
                auto cmem    = *reinterpret_cast<const int*>(&insn.bytes[1]);
                returnTarget = insn.address + insn.size + cmem;
            } else {
                for(const auto& ins : entrypointInstructions)
                    assembler.push_back(ins.toString());
            }
        }
    }

    assembler.push_back("push rax");
    assembler.push_back("mov rax, {}", Assembler::Address(returnTarget));
    assembler.push_back("xchg [rsp], rax");
    assembler.push_back("ret");

    size_t allocationSize = 0x1000;
    originalEntryPointThunk.reset(Allocator{}.allocate(allocationSize));
    assembler.assemble(originalEntryPointThunk.get(), allocationSize);
}

#endif
