#include "AOBScanner.h"
#include "StringUtil.h"
#include <algorithm>
#include <charconv>
#include <optional>
#include <string>

using namespace B3L;

namespace {

    // Contains an even number of hex characters.
    bool isHexString(const std::string& str) {
        if(str.empty())
            return false;

        if(str.size() % 2 != 0)
            return false;

        for(const auto& c : str) {
            if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
                continue;
            return false;
        }
        return true;
    }

} // namespace

bool AOBPattern::isValidPatternString(const std::string& str) {
    if(str.empty())
        return false;

    if(str.size() % 2 != 0)
        return false;

    for(size_t pos = 0; pos < str.size(); ++pos) {
        const char c = str[pos];
        if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
            continue;

        if(c == '?') { // Wild cards have to be of the form "??" with even alignment
            if(pos % 2 != 0 || str[pos + 1] != '?')
                return false;

            ++pos;
            continue;
        }
        return false;
    }
    return true;
}

std::optional<AOBPattern> AOBPattern::fromString(const std::string& str) {
    auto patternString = str;

    StringUtil::removeWhitespace(patternString);

    if(!isValidPatternString(patternString))
        return std::nullopt;

    AOBPattern pattern;
    pattern._bytes.resize(patternString.size() / 2);

    for(size_t pos = 0; pos < patternString.size(); pos += 2) {
        auto it = patternString.data() + pos;

        if(*it == '?')
            pattern._bytes[pos / 2] = -1;
        else
            std::from_chars(it, it + 2, pattern._bytes[pos / 2], 16);
    }
    return pattern;
}

std::vector<int16_t>::const_iterator AOBPattern::begin() const noexcept {
    return _bytes.begin();
}

std::vector<int16_t>::const_iterator AOBPattern::end() const noexcept {
    return _bytes.end();
}

std::optional<std::vector<uint8_t>> B3L::parseByteArrayString(const std::string& str) {
    auto hexString = str;
    StringUtil::removeWhitespace(hexString);

    if(!isHexString(hexString))
        return std::nullopt;

    std::vector<uint8_t> bytes;
    bytes.resize(hexString.size() / 2);

    for(size_t pos = 0; pos < hexString.size(); pos += 2) {
        auto it = hexString.data() + pos;
        std::from_chars(it, it + 2, bytes[pos / 2], 16);
    }

    return bytes;
}
