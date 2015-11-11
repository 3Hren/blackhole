#pragma once

#ifdef __APPLE__
#   include <libkern/OSAtomic.h>
#else
#   include <pthread.h>
#endif

namespace blackhole {
namespace detail {

#if __APPLE__
class spinlock_t {
    OSSpinLock mutex;

public:
    spinlock_t():
        mutex(0)
    {}

    auto lock() -> void {
        OSSpinLockLock(&mutex);
    }

    auto unlock() -> void {
        OSSpinLockUnlock(&mutex);
    }

    auto trylock() -> bool {
        return OSSpinLockTry(&mutex);
    }
};

#else // __APPLE__

class spinlock_t {
    pthread_spinlock_t mutex;

public:
    spinlock_t() {
        pthread_spin_init(&mutex, PTHREAD_PROCESS_PRIVATE);
    }

    ~spinlock_t() {
        pthread_spin_destroy(&mutex);
    }

    auto lock() -> void {
        pthread_spin_lock(&mutex);
    }

    auto unlock() -> void {
        pthread_spin_unlock(&mutex);
    }
};
#endif // __APPLE__

}  // namespace detail
}  // namespace blackhole
