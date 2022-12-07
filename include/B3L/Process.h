#pragma once
#include <cinttypes>
#include <string>

namespace B3L {

    // Returns base address of loaded module, nullptr on failure
    [[nodiscard]] uint8_t* getModuleBaseAddress(const std::string& moduleName);
    [[nodiscard]] uint8_t* getModuleBaseAddress(const char* moduleName = nullptr);

    // Returns filename of executable that was used to load the current process.
    [[nodiscard]] std::string getExecutableFileName() noexcept;

} // namespace B3L
