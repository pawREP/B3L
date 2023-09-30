#pragma once

#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)

#define ANONYMOUS CONCAT(_anonymous, __COUNTER__)

#define B3L_MAKE_NONCOPYABLE(cls)        \
private:                                 \
    cls(const cls&)            = delete; \
    cls& operator=(const cls&) = delete

#define B3L_MAKE_DEFAULTCOPYABLE(cls)     \
public:                                   \
    cls(const cls&)            = default; \
    cls& operator=(const cls&) = default

#define B3L_MAKE_NONMOVABLE(cls)    \
private:                            \
    cls(cls&&)            = delete; \
    cls& operator=(cls&&) = delete

#define B3L_MAKE_DEFAULTMOVABLE(cls)          \
public:                                       \
    cls(cls&&) noexcept            = default; \
    cls& operator=(cls&&) noexcept = default

#define B3L_UNREFERENCED_PARAMETER(P) (P)

#if _MSC_VER
    #define B3L_FORCEINLINE __forceinline
    #define B3L_NEVERINLINE __declspec(noinline)
    #if _MSC_VER >= 1929
        #define B3L_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
    #else
        #define B3L_NO_UNIQUE_ADDRESS
    #endif
#else
    #define B3L_FORCEINLINE __attribute__((always_inline))
    #define B3L_NEVERINLINE __attribute__((noinline))
    #define B3L_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
