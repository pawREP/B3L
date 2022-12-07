#pragma once
#include "Instruction.h"
#include "ScopeExit.h"
#include "capstone/capstone.h"
#include <optional>
#include <vector>

namespace B3L {

    enum class DisassemblerMode : int {
        x86 = CS_MODE_32,
        x64 = CS_MODE_64,
#if _WIN64
        native = x64,
#elif _WIN32
        native = x86,
#endif
    };

    template <DisassemblerMode mode = DisassemblerMode::native>
    class Disassembler {
    public:
        static std::vector<Instruction> disassemble(const uint8_t* code, size_t size, uintptr_t address, size_t count = 0);
        static std::optional<Instruction> disassemble(const uint8_t** code, size_t& size, uintptr_t& address);

    private:
        static csh getHandle();
    };

    template <DisassemblerMode mode = DisassemblerMode::native>
    class StreamDisassembler {
    public:
        StreamDisassembler(const uint8_t* encoding, size_t size, uintptr_t address);

        [[nodiscard]] std::optional<Instruction> read();
        [[nodiscard]] std::optional<Instruction> peek() const;

        [[nodiscard]] uintptr_t tell() const;
        bool seek(uintptr_t address);

    private:
        const uint8_t* code;
        size_t size;
        uintptr_t address;
    };

} // namespace B3L
