#include "Exception.h"
#include <Windows.h>
#include <exception>
#include <format>
#include <string>

Win32Exception::Win32Exception(const std::string& apiName) : Win32Exception(apiName.c_str()) {
}

Win32Exception::Win32Exception(const char* apiName) {
    auto errorId = GetLastError();
    if(errorId == 0) {
        message = std::format("Win32 Exception:\n\t{}", apiName);
        return;
    }

    auto lastErrorMessage = getErrorMessageFromErrorCode(errorId);
    message = std::format("Win32 Exception:\n\tAPI: {}\n\tID:  {}\n\tMsg: {}", apiName, errorId, lastErrorMessage);
}

const char* Win32Exception::what() const {
    return message.c_str();
}

std::string Win32Exception::getErrorMessageFromErrorCode(uint32_t error) {
    // TODO: Maybe move this somewhere else, could be useful outside if this class
    LPSTR messageBuffer = nullptr;
    auto size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                               nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);

    std::string errorMessage(messageBuffer, size);
    LocalFree(messageBuffer);

    return errorMessage;
}
