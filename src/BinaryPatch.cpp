#include "BinaryPatch.h"
#include "Assembler.h"
#include "Cast.h"
#include "Memory.h"

B3L::BinaryPatch::BinaryPatch(uint8_t* entrypoint, const std::string& assembly) : entrypoint(entrypoint) {
    Assembler::Assembler<> assembler;
    assembler.push_back(assembly);
    auto byteCode = assembler.assemble(rcast<uintptr_t>(entrypoint));
    assert(byteCode.size());

    originalEntrypointBytes.resize(byteCode.size());
    std::copy_n(entrypoint, originalEntrypointBytes.size(), originalEntrypointBytes.begin());

    auto oldProtection = Memory::setPageProtection(entrypoint, byteCode.size(), PAGE_EXECUTE_READWRITE);
    std::copy_n(byteCode.begin(), byteCode.size(), entrypoint);
    Memory::setPageProtection(entrypoint, byteCode.size(), oldProtection);
}

B3L::BinaryPatch::BinaryPatch(BinaryPatch&& other) noexcept
: entrypoint(other.entrypoint), originalEntrypointBytes(std::move(other.originalEntrypointBytes)) {
    other.entrypoint = nullptr;
}

B3L::BinaryPatch& B3L::BinaryPatch::operator=(BinaryPatch&& other) noexcept {
    if(isEnabled())
        disable();

    entrypoint              = other.entrypoint;
    originalEntrypointBytes = std::move(other.originalEntrypointBytes);

    other.entrypoint = nullptr;

    return *this;
}

B3L::BinaryPatch::~BinaryPatch() {
    if(entrypoint && isEnabled())
        disable();
}

bool B3L::BinaryPatch::isEnabled() const {
    return std::equal(originalEntrypointBytes.begin(), originalEntrypointBytes.end(), entrypoint);
}

void B3L::BinaryPatch::disable() const {
    auto oldProtect = Memory::setPageProtection(entrypoint, originalEntrypointBytes.size(), PAGE_EXECUTE_READWRITE);
    std::copy_n(originalEntrypointBytes.begin(), originalEntrypointBytes.size(), entrypoint);
    Memory::setPageProtection(entrypoint, originalEntrypointBytes.size(), oldProtect);
}
