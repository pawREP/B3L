#include "Process.h "
#include <Windows.h>
#include <filesystem>

uint8_t* B3L::getModuleBaseAddress(const std::string& moduleName) {
    return getModuleBaseAddress(moduleName.c_str());
}

uint8_t* B3L::getModuleBaseAddress(const char* moduleName) {
    auto handle = GetModuleHandleA(moduleName);
    return reinterpret_cast<uint8_t*>(handle);
}

std::string B3L::getExecutableFileName() noexcept {
    char buf[MAX_PATH];
    if(!GetModuleFileNameA(nullptr, buf, MAX_PATH))
        return "";

    std::filesystem::path executablePath(buf);
    return executablePath.filename().string();
}