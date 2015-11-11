#pragma once

#ifdef __APPLE__
#   include <libkern/OSAtomic.h>
#elif __linux__
#   include <pthread.h>
#endif

namespace blackhole {
namespace detail {

#if __APPLE__
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

#elif __linux__

class spinlock_t {
    pthread_spinlock_t mutex;

public:
    spinlock_t() {
        const int rc = pthread_spin_init(&mutex, PTHREAD_PROCESS_PRIVATE);
        if (rc != 0) {
            throw std::system_error(errno, std::system_category());
        }
    }

    ~spinlock_t() {
        pthread_spin_destroy(&mutex);
    }

    // These methods may fail only if the mutex is invalid or on attempt to use it recursively. We
    // don't do this.
    auto lock() noexcept -> void {
        pthread_spin_lock(&mutex);
    }

    auto unlock() noexcept -> void {
        pthread_spin_unlock(&mutex);
    }
};
#endif

}  // namespace detail
}  // namespace blackhole
