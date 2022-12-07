#include "Debug.h"
#include <Windows.h>

void B3L::detail::debugPrint(const std::string& msg) {
    OutputDebugStringA(msg.c_str());
}