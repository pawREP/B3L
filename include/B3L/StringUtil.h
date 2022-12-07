#pragma once
#include <algorithm>
#include <string>

namespace B3L {
    namespace StringUtil {

        // Replace all substring matches
        void replace(std::string& str, const std::string& old_value, const std::string& new_value);

        // Remove all leading and trailing whitespace characters.
        void trim(std::string& str);

        // Remove all whitespace characters.
        void removeWhitespace(std::string& str);

        // Compare strings case-insensitively
        bool iequal(const std::string& a, const std::string& b);
        bool iequal(const std::string& a, const char* b);
        bool iequal(const char* a, const std::string& b);
        bool iequal(const char* a, const char* b);

    } // namespace StringUtil
} // namespace B3L
