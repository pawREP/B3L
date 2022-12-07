#include "Assembler.h"
#include "Cast.h"
#include "ScopeExit.h"
#include "keystone/keystone.h"
#include <format>

using namespace B3L;

template <Assembler::Mode mode>
size_t Assembler::Assembler<mode>::assemble(uint8_t** encoding, uintptr_t address) {
    static ks_engine* ks = nullptr;

    if(!ks) {
        if(ks_open(KS_ARCH_X86, to_underlying(mode), &ks) != KS_ERR_OK)
            throw std::runtime_error(std::format("ks_open failed. Err: {}", ks_strerror(ks_errno(ks))).c_str());
    }

    size_t size;
    size_t count;

    if(ks_asm(ks, assembly().c_str(), address, encoding, &size, &count) != KS_ERR_OK)
        throw std::runtime_error(std::format("ks_asm failed. Err: {}", ks_strerror(ks_errno(ks))).c_str());

    clear();

    return size;
}

template <Assembler::Mode mode>
std::vector<uint8_t> Assembler::Assembler<mode>::assemble(uintptr_t address) {
    uint8_t* encoding = nullptr;
    auto size         = assemble(&encoding, address);

    std::vector<uint8_t> vec(encoding, encoding + size);

    ks_free(encoding);

    return vec;
}

template <Assembler::Mode mode>
size_t Assembler::Assembler<mode>::assemble(uint8_t* buffer, size_t& size) {
    // TODO: This api is broken, assmebly is cleared on success  of sub call to assemble. If the size is not big enough we dont get another chance
    auto encoding = assemble(rcast<uintptr_t>(buffer));
    if(size < encoding.size()) {
        size = encoding.size();
        return 0;
    }

    std::copy_n(encoding.begin(), encoding.size(), buffer);

    return encoding.size();
}

template <Assembler::Mode mode>
const std::string& Assembler::Assembler<mode>::assembly() const noexcept {
    return _assembly;
}

template <Assembler::Mode mode>
void Assembler::Assembler<mode>::clear() noexcept {
    _assembly.clear();
}

template class Assembler::Assembler<Assembler::Mode::x86>;
template class Assembler::Assembler<Assembler::Mode::x64>;
