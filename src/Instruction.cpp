#ifdef B3L_HAVE_ASSEMBLERS
    #include "Instruction.h"
    #include <format>

using namespace B3L;

Instruction::Instruction(const cs_insn* insn) {
    id             = static_cast<x86_insn>(insn->id);
    address        = insn->address;
    size           = insn->size;
    bytes          = { insn->bytes, insn->bytes + insn->size };
    mnemonic       = insn->mnemonic;
    operandsString = insn->op_str;

    // cs_insn::detail
    if(insn->detail) {

        regsRead  = { insn->detail->regs_read, insn->detail->regs_read + insn->detail->regs_read_count };
        regsWrite = { insn->detail->regs_write, insn->detail->regs_write + insn->detail->regs_write_count };
        groups    = { insn->detail->groups, insn->detail->groups + insn->detail->groups_count };

        // cs_insn::detail::cs_x86
        std::copy(std::begin(insn->detail->x86.prefix), std::end(insn->detail->x86.prefix), prefix.begin());

        {
            auto end = std::find(std::begin(insn->detail->x86.opcode), std::end(insn->detail->x86.opcode), 0);
            opcode   = { std::begin(insn->detail->x86.opcode), end };
        }

        rex      = insn->detail->x86.rex;
        addrSize = insn->detail->x86.addr_size;
        modrm    = insn->detail->x86.modrm;
        sib      = insn->detail->x86.sib;
        disp     = insn->detail->x86.disp;

        sibIndex = insn->detail->x86.sib_index;
        sibScale = insn->detail->x86.sib_scale;
        sibBase  = insn->detail->x86.sib_base;

        xopCc = insn->detail->x86.xop_cc;
        sseCc = insn->detail->x86.sse_cc;
        avxCc = insn->detail->x86.avx_cc;

        avxSae = insn->detail->x86.avx_sae;
        avxRm  = insn->detail->x86.avx_rm;

        eflags = insn->detail->x86.eflags;

        operands = { std::begin(insn->detail->x86.operands), std::begin(insn->detail->x86.operands) + insn->detail->x86.op_count };
        encoding = insn->detail->x86.encoding;
    }
}

std::string Instruction::toString() const {
    return std::format("{} {}", mnemonic, operandsString);
}

#endif
