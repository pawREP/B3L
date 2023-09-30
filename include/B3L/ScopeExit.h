#pragma once
#include "Define.h"
#include <concepts>
#include <exception>

namespace B3L {

#define SCOPE_EXIT const auto ANONYMOUS = detail::MakeScopeExitHelper{} += [&]()
#define SCOPE_SUCCESS const auto ANONYMOUS = detail::MakeScopeSuccessHelper{} += [&]()
#define SCOPE_FAILURE const auto ANONYMOUS = detail::MakeScopeFailureHelper{} += [&]()

    namespace detail {

        struct ScopeExitExecCondition {};
        struct ScopeExitAlways : ScopeExitExecCondition {};
        struct ScopeExitSuccess : ScopeExitExecCondition {};
        struct ScopeExitFailure : ScopeExitExecCondition {};

        template <std::invocable Callable, std::derived_from<ScopeExitExecCondition> Condition>
        class ScopeExit {
            // Copy and move could be supported but it seems antithetical to the intended use case.
            B3L_MAKE_NONCOPYABLE(ScopeExit);
            B3L_MAKE_NONMOVABLE(ScopeExit);

        public:
            explicit ScopeExit(Callable&& callable) noexcept(std::is_nothrow_move_constructible_v<Callable>)
            : callable(callable) {
            }

            ~ScopeExit() {
                if constexpr(std::is_same_v<Condition, ScopeExitFailure>) {
                    if(!std::uncaught_exceptions())
                        return;
                } else if(std::is_same_v<Condition, ScopeExitSuccess>) {
                    if(std::uncaught_exceptions())
                        return;
                }

                callable();
            }

        private:
            // Prevent construction on the heap
            void* operator new(size_t) {
                return nullptr;
            };
            void* operator new(size_t, void*) {
                return nullptr;
            };
            void* operator new[](size_t) {
                return nullptr;
            };
            void* operator new[](size_t, void*) {
                return nullptr;
            };

            Callable callable;
        };

    } // namespace detail

    // Those specializations are ugly but we can't do type aliases or inherit constructors without losing type deduction :/

    // Owning wrapper around callable. Unconditionally executes callable on scope exit.
    template <std::invocable Callable>
    struct ScopeExit : public detail::ScopeExit<Callable, detail::ScopeExitAlways> {
        explicit ScopeExit(Callable&& callable)
        : detail::ScopeExit<Callable, detail::ScopeExitAlways>(std::forward<Callable&&>(callable)){};
    };

    // Owning wrapper around callable. Executes callable on scope exit if no exceptions are in flight.
    template <std::invocable Callable>
    struct ScopeSuccess : public detail::ScopeExit<Callable, detail::ScopeExitSuccess> {
        explicit ScopeSuccess(Callable&& callable)
        : detail::ScopeExit<Callable, detail::ScopeExitSuccess>(std::forward<Callable&&>(callable)){};
    };

    // Owning wrapper around callable. Executes callable on scope exit if at least one exception is in flight.
    template <std::invocable Callable>
    struct ScopeFailure : public detail::ScopeExit<Callable, detail::ScopeExitFailure> {
        explicit ScopeFailure(Callable&& callable)
        : detail::ScopeExit<Callable, detail::ScopeExitFailure>(std::forward<Callable&&>(callable)){};
    };

    namespace detail {

        struct MakeScopeExitHelper {
            template <typename F>
            B3L::ScopeExit<F> operator+=(F&& fn) {
                return B3L::ScopeExit<F>(std::move(fn));
            }
        };

        struct MakeScopeSuccessHelper {
            template <typename F>
            B3L::ScopeSuccess<F> operator+=(F&& fn) {
                return B3L::ScopeSuccess<F>(std::move(fn));
            }
        };

        struct MakeScopeFailureHelper {
            template <typename F>
            B3L::ScopeFailure<F> operator+=(F&& fn) {
                return B3L::ScopeFailure<F>(std::move(fn));
            }
        };

    } // namespace detail

} // namespace B3L
