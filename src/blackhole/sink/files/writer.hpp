#pragma once

#include <string>

#include "blackhole/detail/config/noncopyable.hpp"
#include "blackhole/error.hpp"

namespace blackhole {

namespace sink {

namespace files {

template<class Backend>
class writer_t {
    BLACKHOLE_DECLARE_NONCOPYABLE(writer_t);

public:
    typedef Backend backend_type;

private:
    backend_type& backend;

public:
    writer_t(backend_type& backend) :
        backend(backend)
    {}

    void write(const std::string& message) {
        if (!backend.opened()) {
            if (!backend.open()) {
                throw error_t("failed to open file '%s' for writing", backend.path());
            }
        }
        backend.write(message);
    }
};

} // namespace files

} // namespace sink

} // namespace blackhole
