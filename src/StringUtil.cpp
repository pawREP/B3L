#include "StringUtil.h"

void B3L::StringUtil::replace(std::string& str, const std::string& old_value, const std::string& new_value) {
    if(str.empty() || old_value.empty())
        return;

    size_t pos = 0;
    while((pos = str.find(old_value, pos)) != std::string::npos) {
        str.replace(pos, old_value.size(), new_value);
        pos += new_value.size();
    }
}

void B3L::StringUtil::trim(std::string& str) {
    auto leading = std::find_if(str.begin(), str.end(), [](char c) { return !iswspace(c); });
    if(leading != str.end())
        str.erase(str.begin(), leading);

    auto trailing = std::find_if(str.rbegin(), str.rend(), [](char c) { return !iswspace(c); });
    if(trailing != str.rend())
        str.erase(std::next(trailing).base(), str.end());
}

void B3L::StringUtil::removeWhitespace(std::string& str) {
    std::erase_if(str, [](char c) { return iswspace(c); });
}

bool B3L::StringUtil::iequal(const std::string& a, const std::string& b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) { return tolower(a) == tolower(b); });
}

bool B3L::StringUtil::iequal(const std::string& a, const char* b) {
    return std::equal(a.begin(), a.end(), b, b + strlen(b), [](char a, char b) { return tolower(a) == tolower(b); });
}

bool B3L::StringUtil::iequal(const char* a, const std::string& b) {
    return std::equal(b.begin(), b.end(), a, a + strlen(a), [](char a, char b) { return tolower(a) == tolower(b); });
}

bool B3L::StringUtil::iequal(const char* a, const char* b) {
    return std::equal(a, a + strlen(a), b, b + strlen(b), [](char a, char b) { return tolower(a) == tolower(b); });
}
