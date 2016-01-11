#pragma once

#include <libkern/OSAtomic.h>

namespace blackhole {
inline namespace v1 {
namespace detail {

class spinlock_t {
    OSSpinLock mutex;

public:
    constexpr spinlock_t() noexcept:
        mutex(0)
    {}

    auto lock() noexcept -> void {
        OSSpinLockLock(&mutex);
    }

    auto unlock() noexcept -> void {
        OSSpinLockUnlock(&mutex);
    }

    auto trylock() noexcept -> bool {
        return OSSpinLockTry(&mutex);
    }
};

}  // namespace detail
}  // namespace v1
}  // namespace blackhole
