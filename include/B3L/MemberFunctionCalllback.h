#pragma once
#include <functional>
#include <stdexcept>

namespace B3L {

    template <typename>
    struct member_function;

    template <typename Ret, typename Object, typename... Param>
    struct member_function<Ret (Object::*)(Param...)> {
        using object_type             = Object;
        using return_type             = Ret;
        using signature               = Ret (Object::*)(Param...);
        using free_signature          = Ret (*)(Param...);
        using explicit_this_signature = Ret (*)(Object*, Param...);
    };

    // Transforms a member function signature to the corresponding free function signature with explicit this parameter.
    template <class T>
    using add_explicit_this_t = typename member_function<T>::explicit_this_signature;

    // Transforms a member function signature to the apparent free function signature.
    template <class T>
    using as_free_function_t = typename member_function<T>::free_signature;

    namespace detail {

        template <size_t, typename>
        struct MemberFunctionCallback;

        template <size_t ID, typename Ret, typename Object, typename... Params>
        struct MemberFunctionCallback<ID, Ret (Object::*)(Params...)> {
            static Ret callback(Params... args) {
                return func(args...);
            }
            static inline std::function<Ret(Params...)> func = nullptr;
        };

    } // namespace detail

    // Binds a class instance to a member function pointer and returns the pairing as a free function pointer with the
    // same signature. Multiple instances can be uniquely bound by instatiating this function with unique IDs.
    template <size_t ID = 0, typename MemFn = std::false_type>
    auto bindInstance(typename member_function<MemFn>::object_type* this_, MemFn memberFunction) {
        static_assert(std::is_member_function_pointer_v<MemFn>);

        if(detail::MemberFunctionCallback<ID, MemFn>::func)
            throw std::runtime_error("Tried to bind twice to same member function callback delegate");

        detail::MemberFunctionCallback<ID, MemFn>::func = std::bind_front(memberFunction, this_);
        return detail::MemberFunctionCallback<ID, MemFn>::callback;
    }

} // namespace B3L

#define B3L_MAKE_UNIQUE_MEMBERFUNCTION_CALLBACK(instance, memberfunction) \
    B3L::bindInstance<__COUNTER__>(instance, memberfunction)
