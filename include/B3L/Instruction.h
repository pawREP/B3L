#pragma once
#include "capstone/capstone.h"
#include <array>
#include <string>
#include <vector>

namespace B3L {

    class Instruction {
    public:
        Instruction() = default;
        explicit Instruction(const cs_insn* insn);

        Instruction(Instruction&&) noexcept            = default;
        Instruction(const Instruction&)                = default;
        ~Instruction()                                 = default;
        Instruction& operator=(Instruction&&) noexcept = default;
        Instruction& operator=(const Instruction&)     = default;

        // cs_insn
        x86_insn id = x86_insn::X86_INS_INVALID;
        uint64_t address{};
        uint16_t size{};
        std::vector<uint8_t> bytes;
        std::string mnemonic;
        std::string operandsString;

        // cs_insn::detail
        std::vector<uint16_t> regsRead;
        std::vector<uint16_t> regsWrite;
        std::vector<uint8_t> groups;

        // cs_insn::detail::cs_x86
        std::array<uint8_t, 4> prefix;
        std::vector<uint8_t> opcode;

        uint8_t rex{};
        uint8_t addrSize{};
        uint8_t modrm{};
        uint8_t sib{};
        int64_t disp{};

        x86_reg sibIndex = X86_REG_INVALID;
        int8_t sibScale{};
        x86_reg sibBase = X86_REG_INVALID;

        x86_xop_cc xopCc = X86_XOP_CC_INVALID;
        x86_sse_cc sseCc = X86_SSE_CC_INVALID;
        x86_avx_cc avxCc = X86_AVX_CC_INVALID;

        bool avxSae      = false;
        x86_avx_rm avxRm = X86_AVX_RM_INVALID;

        union {
            uint64_t eflags;
            uint64_t fpu_flags;
        };

        std::vector<cs_x86_op> operands;
        cs_x86_encoding encoding{};

        std::string toString() const;
    };

} // namespace B3L
