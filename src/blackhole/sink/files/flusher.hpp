#pragma once

#include "blackhole/detail/config/noncopyable.hpp"

namespace blackhole {

namespace sink {

namespace files {

template<class Backend>
class flusher_t {
    BLACKHOLE_DECLARE_NONCOPYABLE(flusher_t);

public:
    typedef Backend backend_type;

private:
    const bool autoflush;
    backend_type& backend;

public:
    flusher_t(bool autoflush, backend_type& backend) :
        autoflush(autoflush),
        backend(backend)
    {}

    void flush() {
        if (autoflush) {
            backend.flush();
        }
    }
};

} // namespace files

} // namespace sink

} // namespace blackhole
