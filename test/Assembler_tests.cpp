#include "B3L/Assembler.h"
#include <gtest/gtest.h>

using namespace B3L::Assembler;

TEST(AssemblerTest, AssembleX64) {
    Assembler<Mode::x64> assembler;
    assembler.push_back("xchg rax, [rsp]");

    auto encoding = assembler.assemble(0);

    uint8_t soll[] = { 0x48, 0x87, 0x04, 0x24 };

    EXPECT_EQ(encoding.size(), sizeof(soll));
    EXPECT_TRUE(std::equal(std::begin(soll), std::end(soll), encoding.begin()));
    EXPECT_EQ(assembler.assembly().size(), 0);
}

TEST(AssemblerTest, AssembleX86) {
    Assembler<Mode::x86> assembler;
    assembler.push_back("xchg eax, [esp]");
    auto encoding = assembler.assemble(0);

    const uint8_t soll[] = { 0x87, 0x04, 0x24 };

    EXPECT_EQ(encoding.size(), sizeof(soll));
    EXPECT_TRUE(std::equal(std::begin(soll), std::end(soll), encoding.begin()));
}

TEST(AssemblerTest, AssembleError) {
    Assembler<Mode::x64> assembler;
    assembler.push_back("push abc"); // invalid insn;

    EXPECT_THROW(
    {
        try {
            auto encoding = assembler.assemble(0);
        } catch(...) {
            // assembly does not get cleared on error
            EXPECT_NE(assembler.assembly().size(), 0);
            throw;
        }
        EXPECT_TRUE(false);
    },
    std::exception);
}

TEST(AssemblerTest, UseImmediates) {
    int a{};
    Assembler<Mode::x64> assembler;

    assembler.push_back("mov rax, {}", Imm64{ 0x140000000 });
    assembler.push_back("mov al, {}", Imm8{ 0x01 });
    assembler.push_back("lea rax, [{}]", Address{ 0x140000000 });
    assembler.push_back("lea rax, [{}]", Address{ &a });
    assembler.push_back("lea rax, [rax + {}]", Imm32{ 0x1000 });
    assembler.push_back("mov rax, {}", Float{ 1.f });

    auto encoding = assembler.assemble(0);
}
