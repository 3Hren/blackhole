#pragma once

#include <os/lock.h>

namespace blackhole {
inline namespace v1 {
namespace detail {

class spinlock_t {
    os_unfair_lock mutex;

public:
    constexpr spinlock_t() noexcept:
        mutex({0})
    {}

    auto lock() noexcept -> void {
        os_unfair_lock_lock(&mutex);
    }

    auto unlock() noexcept -> void {
        os_unfair_lock_unlock(&mutex);
    }

    auto trylock() noexcept -> bool {
        return os_unfair_lock_trylock(&mutex);
    }
};

}  // namespace detail
}  // namespace v1
}  // namespace blackhole
