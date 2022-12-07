#include <format>

namespace B3L {

    namespace detail {

        void debugPrint(const std::string& msg);

    } // namespace detail

    // Passes formated string to debugger
    template <typename... Args>
    void debugPrint(std::format_string<Args...> fmt, Args&&... args) {
        detail::debugPrint(std::format(fmt, std::forward<Args>(args)...));
    }

} // namespace B3L
