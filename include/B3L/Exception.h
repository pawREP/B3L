#pragma once
#include <string>

class Win32Exception : public std::exception {
public:
    Win32Exception(const char* apiName);
    Win32Exception(const std::string& apiName);

    [[nodiscard]] const char* what() const override;

private:
    static std::string getErrorMessageFromErrorCode(uint32_t error);

    std::string message; // TODO: Look into exception allocation issue
};