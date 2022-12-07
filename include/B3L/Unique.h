#pragma once
#include "Define.h"
#include <mutex>

namespace B3L {
    // CRTP base class which enforces that only one unique instance of the derived class can be alive at any given time.
    template <typename CRPT>
    class Unique {
        // TODO:  Unique causes issues on destruction when compiled with clang. Needs to be investigated
        B3L_MAKE_NONCOPYABLE(Unique);

    protected:
        Unique() : _lock(_mutex, std::defer_lock) {
            if(!_lock.try_lock())
                throw std::runtime_error("Attempted to create multiple instances of unique type");
        }
        Unique(Unique&&) noexcept            = default;
        Unique& operator=(Unique&&) noexcept = default;
        ~Unique()                            = default;

    private:
        static inline std::mutex _mutex{};
        std::unique_lock<std::mutex> _lock;
    };

} // namespace B3L
