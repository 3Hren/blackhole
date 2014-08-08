#pragma once

#include <mutex>
#include <tuple>

#include <boost/thread.hpp>

namespace blackhole {

namespace detail {

namespace thread {

namespace multi_lock {

template<class... Mutex>
class helper_t {
    std::tuple<std::unique_lock<Mutex>...> locks;

public:
    helper_t(Mutex&... mutexes) {
        boost::lock(mutexes...);
        locks = std::make_tuple(std::unique_lock<Mutex>(mutexes, std::adopt_lock)...);
    }

    helper_t(helper_t&& other) :
        locks(std::move(other.locks))
    {}
};

} // namespace multi_lock

template<class... Mutex>
inline
multi_lock::helper_t<Mutex...> make_multi_lock_t(Mutex&... mutexes) {
    return multi_lock::helper_t<Mutex...>(mutexes...);
}

} // namespace thread

} // namespace detail

} // namespace blackhole
