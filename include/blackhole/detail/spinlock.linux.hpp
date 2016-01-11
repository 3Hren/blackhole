#pragma once

#include <pthread.h>

namespace blackhole {
inline namespace v1 {
namespace detail {

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

}  // namespace detail
}  // namespace v1
}  // namespace blackhole
