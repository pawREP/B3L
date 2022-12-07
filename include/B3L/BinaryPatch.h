#pragma once
#include "Define.h"
#include "Memory.h"
#include <Windows.h>
#include <string>
#include <vector>

namespace B3L {

    class BinaryPatch {
        B3L_MAKE_NONCOPYABLE(BinaryPatch);

    public:
        BinaryPatch() = default;

        template <std::forward_iterator FwdIter>
        BinaryPatch(uint8_t* entrypoint, FwdIter begin, FwdIter end);
        BinaryPatch(uint8_t* entrypoint, const std::string& assembly);
        BinaryPatch(BinaryPatch&& other) noexcept;
        BinaryPatch& operator=(BinaryPatch&& other) noexcept;
        ~BinaryPatch();

    private:
        bool isEnabled() const;
        void disable() const;

        uint8_t* entrypoint = nullptr;
        std::vector<uint8_t> originalEntrypointBytes{};
    };

    template <std::forward_iterator FwdIter>
    inline BinaryPatch::BinaryPatch(uint8_t* entrypoint, FwdIter begin, FwdIter end) : entrypoint(entrypoint) {
        auto overwriteSize = std::distance(begin, end);

        originalEntrypointBytes.resize(overwriteSize);
        std::copy_n(entrypoint, originalEntrypointBytes.size(), originalEntrypointBytes.begin());

        auto oldProtection = Memory::setPageProtection(entrypoint, overwriteSize, PAGE_EXECUTE_READWRITE);
        std::copy(begin, end, entrypoint);
        Memory::setPageProtection(entrypoint, overwriteSize, oldProtection);
    }

} // namespace B3L