#ifdef B3L_HAVE_ASSEMBLERS
    #include "B3L/Cast.h"
    #include "B3L/Disassembler.h"
    #include <array>
    #include <gtest/gtest.h>

using namespace B3L;

TEST(DisassemblerTest, AssembleX64) {
    uint8_t code[]     = { 0x50, 0x48, 0x87, 0x04, 0x24 };
    const size_t count = 2;

    std::vector<Instruction> instructions =
    Disassembler<DisassemblerMode::x64>::disassemble(code, sizeof(code), rcast<uintptr_t>(code), 2);

    EXPECT_EQ(instructions.size(), count);
    EXPECT_STREQ(instructions[0].toString().c_str(), "push rax");
    EXPECT_STREQ(instructions[1].toString().c_str(), "xchg qword ptr [rsp], rax");
}

TEST(DisassemblerTest, AssembleX86) {
    uint8_t code[]     = { 0x50, 0x87, 0x04, 0x24 };
    const size_t count = 2;

    std::vector<Instruction> instructions =
    Disassembler<DisassemblerMode::x86>::disassemble(code, sizeof(code), rcast<uintptr_t>(code), 2);

    EXPECT_EQ(instructions.size(), count);
    EXPECT_STREQ(instructions[0].toString().c_str(), "push eax");
    EXPECT_STREQ(instructions[1].toString().c_str(), "xchg dword ptr [esp], eax");
}

#endif
