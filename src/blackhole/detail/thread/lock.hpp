#pragma once

#include <mutex>
#include <tuple>

#include <boost/thread.hpp>

namespace blackhole {

namespace detail {

namespace thread {

template<class... Mutex>
class multi_lock_t {
    std::tuple<std::unique_lock<Mutex>...> locks;

public:
    multi_lock_t(Mutex&... mutexes) {
        boost::lock(mutexes...);
        locks = std::make_tuple(std::unique_lock<Mutex>(mutexes, std::adopt_lock)...);
    }

    multi_lock_t(multi_lock_t&& other) :
        locks(std::move(other.locks))
    {}
};

template<class... Mutex>
static inline multi_lock_t<Mutex...> multi_lock(Mutex&... mutexes) {
    return multi_lock_t<Mutex...>(mutexes...);
}

} // namespace thread

} // namespace detail

} // namespace blackhole
