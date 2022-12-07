#pragma once
#include <algorithm>
#include <optional>
#include <string>
#include <vector>

namespace B3L {

    [[nodiscard]] std::optional<std::vector<uint8_t>> parseByteArrayString(const std::string& byteString);

    struct AOBPattern {
    public:
        [[nodiscard]] static std::optional<AOBPattern> fromString(const std::string& str);

        [[nodiscard]] std::vector<int16_t>::const_iterator begin() const noexcept;
        [[nodiscard]] std::vector<int16_t>::const_iterator end() const noexcept;

    private:
        AOBPattern() = default;

        [[nodiscard]] static bool isValidPatternString(const std::string& str);

        std::vector<int16_t> _bytes;
    };

    class AOBScanner {
    public:
        template <std::forward_iterator FwdIter1>
        static FwdIter1 find(FwdIter1 haystackBegin, FwdIter1 haystackEnd, const AOBPattern& needle) {
            return find(haystackBegin, haystackEnd, needle.begin(), needle.end());
        }

        template <std::forward_iterator FwdIter1, std::forward_iterator FwdIter2>
        static FwdIter1 find(FwdIter1 haystackBegin, FwdIter1 haystackEnd, FwdIter2 needleBegin, FwdIter2 needleEnd) {
            static_assert(std::is_same_v<typename std::iterator_traits<FwdIter1>::value_type, uint8_t>);
            static_assert(std::is_same_v<typename std::iterator_traits<FwdIter2>::value_type, int16_t>);

            static auto pred = [](uint8_t c, int16_t p) {
                if(p < 0)
                    return true;
                return c == static_cast<uint8_t>(p);
            };

            return std::search(haystackBegin, haystackEnd, needleBegin, needleEnd, pred);
        }
    };

} // namespace B3L