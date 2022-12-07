#pragma once
#include "Cast.h"
#include <format>
#include <string>
#include <vector>

namespace B3L {

    namespace Assembler {

        enum class Mode : int {
            x86 = 1 << 2,
            x64 = 1 << 3,
#if _WIN64
            native = x64,
#elif _WIN32
            native = x86,
#endif
        };

        template <Mode mode = Mode::native>
        class Assembler {
        public:
            // Pushes back instruction.
            //
            // Example usage:
            //      push_back("push rax");
            //      push_back("lea rax, [{}]", Imm32{ 0x1234 });
            template <typename... Args>
            void push_back(const char* instruction, Args... args);
            template <typename... Args>
            void push_back(const std::string& instruction, Args... args);

            // Assembles the staged instruction sequence, returns the corresponding encoding and clears the internal
            // assembly buffer. Throws std::exception on failure. The internal state is not cleared on failure.
            std::vector<uint8_t> assemble(uintptr_t address);

            // Assembles staged instructions into user provided buffer and returns size of encoding. Returns 0 on failure.
            // If the failure was caused by and insufficient buffer size, the size parameter is updated to the required size.
            size_t assemble(uint8_t* buffer, size_t& size);

            // Returns the currently staged assembly.
            const std::string& assembly() const noexcept;

            // Clears the internal assembly buffer.
            void clear() noexcept;

        private:
            size_t assemble(uint8_t** encoding, uintptr_t address);

            std::string _assembly;
        };

        template <Mode mode>
        template <typename... Args>
        inline void Assembler<mode>::push_back(const std::string& instruction, Args... args) {
            push_back(instruction.c_str(), std::forward<Args>(args)...);
        }

        template <Mode mode>
        template <typename... Args>
        // inline void Assembler<mode>::push_back(const std::string& instruction, Args... args) {
        inline void Assembler<mode>::push_back(const char* instruction, Args... args) {
            if constexpr(sizeof...(Args) == 0)
                _assembly += instruction;
            else
                // TODO: Could use compile time string as template arg to avoid runtime checks on format string. Would allow use of std::format.
                _assembly += std::vformat(instruction, std::make_format_args(args...));

            _assembly += ';';
        }

        template <typename T>
        struct Imm {
            T value;
        };

        template <>
        struct Imm<uintptr_t> {
            template <std::integral T>
            Imm(T t) {
                value = domain_cast<uintptr_t>(t);
            };
            Imm(const void* t) {
                value = rcast<uintptr_t>(t);
            };
            uintptr_t value;
        };

        template <class T>
        Imm(T a) -> Imm<T>;

        using Imm8    = Imm<int8_t>;
        using Imm16   = Imm<int16_t>;
        using Imm32   = Imm<int32_t>;
        using Imm64   = Imm<int64_t>;
        using Float   = Imm<float>;
        using Double  = Imm<double>;
        using Address = Imm<uintptr_t>;

    } // namespace Assembler

} // namespace B3L

template <typename T>
struct std::formatter<B3L::Assembler::Imm<T>> : std::formatter<uint64_t> {
    template <typename FormatContext>
    auto format(const B3L::Assembler::Imm<T>& imm, FormatContext& ctx) const {
        if constexpr(std::is_floating_point_v<T>)
            if constexpr(std::is_same_v<T, float>)
                return std::format_to(ctx.out(), "{:#x}", B3L::bit_cast<uint32_t>(imm.value));
            else
                return std::format_to(ctx.out(), "{:#x}", B3L::bit_cast<uint64_t>(imm.value));
        else
            return std::format_to(ctx.out(), "{:#x}", imm.value);
    }
};
