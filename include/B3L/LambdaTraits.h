#pragma once

template <class F>
struct lambda_traits : lambda_traits<decltype(&F::operator())> {};

template <class F, class R, class... Args>
struct lambda_traits<R (F::*)(Args...) const> {
    using pointer = typename std::add_pointer<R(Args...)>::type;
};
