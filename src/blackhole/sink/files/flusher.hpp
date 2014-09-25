#pragma once

namespace blackhole {

namespace sink {

namespace files {

template<class Backend>
class flusher_t {
public:
    typedef Backend backend_type;

private:
    bool autoflush;
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
