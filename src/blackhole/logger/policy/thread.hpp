#pragma once

#include <mutex>

#include <boost/thread/shared_mutex.hpp>

#include "blackhole/detail/config/inline.hpp"

namespace blackhole {

namespace policy {

namespace threading {

struct null_mutex_t {
    BLACKHOLE_ALWAYS_INLINE void lock() {}
    BLACKHOLE_ALWAYS_INLINE bool try_lock() { return true; }
    BLACKHOLE_ALWAYS_INLINE void unlock() {}
};

struct null_t {
    typedef null_mutex_t                  rw_mutex_type;
    typedef std::lock_guard<null_mutex_t> reader_lock_type;
    typedef std::lock_guard<null_mutex_t> writer_lock_type;
};

struct rw_lock_t {
    typedef boost::shared_mutex               rw_mutex_type;
    typedef boost::shared_lock<rw_mutex_type> reader_lock_type;
    typedef boost::unique_lock<rw_mutex_type> writer_lock_type;
};

} // namespace threading

} // namespace policy

} // namespace blackhole
