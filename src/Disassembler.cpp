#include "Disassembler.h"
#include "Cast.h"
#include <cassert>
#include <format>
#include <vector>

template <B3L::DisassemblerMode mode>
csh B3L::Disassembler<mode>::getHandle() {
    static csh handle{};

    if(!handle) {
        auto err = cs_open(CS_ARCH_X86, scast<cs_mode>(mode), &handle);
        if(err != CS_ERR_OK)
            throw std::runtime_error(std::format("Failed to initialize Capstone. Err: {}", cs_strerror(err)).c_str());

        cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
    }

    return handle;
}

template <B3L::DisassemblerMode mode>
std::vector<B3L::Instruction>
B3L::Disassembler<mode>::disassemble(const uint8_t* code, size_t size, uintptr_t address, size_t count) {
    cs_insn* insn = nullptr;

    const size_t isCount = cs_disasm(getHandle(), code, size, address, count, &insn);
    if(!isCount)
        return std::vector<Instruction>{};
    
    //const ScopeExit _{ [insn, isCount] { cs_free(insn, isCount); } }; // NOLINT
    const ScopeExit _{ [insn, isCount] { cs_free(insn, isCount); } }; // NOLINT

    std::vector<Instruction> instructions;
    instructions.reserve(isCount);

    for(size_t i = 0; i < isCount; ++i)
        instructions.emplace_back(insn + i);

    return instructions;
}

template <B3L::DisassemblerMode mode>
std::optional<B3L::Instruction> B3L::Disassembler<mode>::disassemble(const uint8_t** code, size_t& size, uintptr_t& address) {
    static cs_insn* insn = cs_malloc(getHandle()); // NOLINT

    if(cs_disasm_iter(getHandle(), code, &size, &address, insn))
        return std::make_optional<Instruction>(insn);

    return std::nullopt;
}

template class B3L::Disassembler<B3L::DisassemblerMode::x86>;
template class B3L::Disassembler<B3L::DisassemblerMode::x64>;

template <B3L::DisassemblerMode mode>
B3L::StreamDisassembler<mode>::StreamDisassembler(const uint8_t* code, size_t size, uintptr_t address)
: code(code), size(size), address(address) {
}

template <B3L::DisassemblerMode mode>
std::optional<B3L::Instruction> B3L::StreamDisassembler<mode>::peek() const {
    // Some local copies for const correctness
    const uint8_t* codeLocal = code;
    size_t sizeLocal         = size;
    uintptr_t addressLocal   = address;

    return Disassembler<mode>::disassemble(&codeLocal, sizeLocal, addressLocal);
}

template <B3L::DisassemblerMode mode>
std::optional<B3L::Instruction> B3L::StreamDisassembler<mode>::read() {
    return Disassembler<mode>::disassemble(&code, size, address);
}

template <B3L::DisassemblerMode mode>
uintptr_t B3L::StreamDisassembler<mode>::tell() const {
    return address;
}

template <B3L::DisassemblerMode mode>
bool B3L::StreamDisassembler<mode>::seek([[maybe_unused]] uintptr_t addr) {

    return false; // TODO:IMpl
}

template class B3L::StreamDisassembler<B3L::DisassemblerMode::x86>;
template class B3L::StreamDisassembler<B3L::DisassemblerMode::x64>;
