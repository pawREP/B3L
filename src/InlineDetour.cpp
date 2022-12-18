#include "InlineDetour.h"
#include "Assembler.h"
#include "Cast.h"
#include "Disassembler.h"
#include "Memory.h"

using namespace B3L;

InlineDetour::InlineDetour(uint8_t* entrypoint, const uint8_t* target) : entrypoint(entrypoint) {
    entrypointInstructions = disassembleEntrypoint(entrypoint, &entrypointSize);
    detourEntrypoint(target);
}

B3L::InlineDetour::InlineDetour(InlineDetour&& other) noexcept
: entrypoint(other.entrypoint), entrypointSize(other.entrypointSize),
  entrypointInstructions(std::move(other.entrypointInstructions)), trampoline(std::move(other.trampoline)) {
}

InlineDetour& B3L::InlineDetour::operator=(InlineDetour&& other) noexcept {
    restoreEntrypoint();

    entrypoint             = other.entrypoint;
    entrypointInstructions = std::move(other.entrypointInstructions);
    entrypointSize         = other.entrypointSize;
    trampoline             = std::move(other.trampoline);

    return *this;
}

B3L::InlineDetour::~InlineDetour() {
    restoreEntrypoint();
}

std::vector<Instruction> InlineDetour::disassembleEntrypoint(uint8_t* entrypoint, size_t* size) {
    StreamDisassembler<> disa(entrypoint, 0x1000, rcast<uintptr_t>(entrypoint));

    std::vector<Instruction> instructions;
    size_t entrypointSize{};
    while(entrypointSize < minEntrypointSize) {
        auto insn = disa.read();
        if(!insn)
            throw std::runtime_error("Failed to disassemble entrypoint");

        entrypointSize += insn->size;
        instructions.emplace_back(std::move(insn.value()));
    }
    *size = entrypointSize;
    return instructions;
}

void B3L::InlineDetour::detourEntrypoint(const uint8_t* target) {
    Assembler::Assembler<> assembler;
    //
    assembler.push_back("push rax");
    assembler.push_back("mov rax, {}", Assembler::Address(target));
    assembler.push_back("xchg [rsp], rax");
    assembler.push_back("ret");

    size_t trampolineBufferSize = 0x1000; // One page
    {
        Allocator allocator{};
        auto mem = allocator.allocate(trampolineBufferSize, entrypoint);
        assert(mem);
        trampoline.reset(mem);
    }
    if(!assembler.assemble(trampoline.get(), trampolineBufferSize))
        throw std::runtime_error("Failed to assemble trampoline");

    assembler.clear();
    //
    assembler.push_back("jmp {}", Assembler::Address{ trampoline.get() });
    auto mem = assembler.assemble(rcast<uintptr_t>(entrypoint));
    if(mem.empty())
        throw std::runtime_error("Failed to assemble new entrypoint");

    // TODO: Suspend

    auto oldProtection = Memory::setPageProtection(entrypoint, mem.size(), PAGE_EXECUTE_READWRITE);
    std::copy_n(mem.begin(), mem.size(), entrypoint);
    Memory::setPageProtection(entrypoint, mem.size(), oldProtection);

    FlushInstructionCache(GetCurrentProcess(), entrypoint, entrypointSize);
}

void B3L::InlineDetour::restoreEntrypoint() {
    if(entrypointInstructions.empty()) // moved from
        return;

    // TODO: Suspend

    auto oldProtection = Memory::setPageProtection(entrypoint, entrypointSize, PAGE_EXECUTE_READWRITE);
    uint8_t* head      = entrypoint;
    for(const auto& insn : entrypointInstructions) {
        std::copy(insn.bytes.begin(), insn.bytes.end(), head);
        head += insn.size;
    }
    Memory::setPageProtection(entrypoint, entrypointSize, oldProtection);
}
