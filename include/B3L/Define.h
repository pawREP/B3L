#pragma once

#define B3L_MAKE_NONCOPYABLE(cls)        \
private:                                 \
    cls(const cls&)            = delete; \
    cls& operator=(const cls&) = delete

#define B3L_MAKE_NONMOVABLE(cls)    \
private:                            \
    cls(cls&&)            = delete; \
    cls& operator=(cls&&) = delete

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
