#include "Memory.h"
#include "Exception.h"

namespace {

    struct AddressRange {
        uintptr_t min;
        uintptr_t max;
    };

    AddressRange getApplicationAddressRange() {
        SYSTEM_INFO info;
        GetSystemInfo(&info);

        return { .min = reinterpret_cast<uintptr_t>(info.lpMinimumApplicationAddress),
                 .max = reinterpret_cast<uintptr_t>(info.lpMaximumApplicationAddress) };
    }

} // namespace

bool B3L::Memory::isInApplicationAddressRange(uintptr_t addr) {
    static AddressRange validAddressRange = getApplicationAddressRange();
    return (addr >= validAddressRange.min) && (addr < validAddressRange.max);
}

bool B3L::Memory::isInApplicationAddressRange(void* addr) {
    return isInApplicationAddressRange(reinterpret_cast<uintptr_t>(addr));
}

int B3L::Memory::setPageProtection(void* addr, size_t size, int protection) {
    DWORD old;
    if(!VirtualProtect(addr, size, protection, &old))
        throw Win32Exception("VirtualProtect");
    return old;
}
