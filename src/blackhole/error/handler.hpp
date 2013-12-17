#pragma once

#include <functional>
#include <stdexcept>

#include "blackhole/config.hpp"

namespace blackhole {

namespace log {

typedef std::function<void()> exception_handler_t;

class default_exception_handler_t {
public:
    void operator()() const {
#ifdef BLACKHOLE_DEBUG
        throw;
#else
        try {
            throw;
        } catch (const std::exception& err) {
            std::cout << "logging core error occurred: " << err.what() << std::endl;
        } catch (...) {
            std::cout << "logging core error occurred: unknown" << std::endl;
        }
#endif
    }
};

} // namespace log

} // namespace blackhole
