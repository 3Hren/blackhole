#pragma once

namespace blackhole {

namespace sink {

namespace files {

template<class Backend>
class flusher_t {
    bool autoflush;
    Backend& backend;
public:
    flusher_t(bool autoflush, Backend& backend) :
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
