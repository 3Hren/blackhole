#pragma once

#include <stdexcept>

#include "utils/format.hpp"

namespace blackhole {

class error_t : public std::runtime_error {
public:
    template<typename... Args>
    error_t(const std::string& reason, Args&&... args) :
        std::runtime_error(utils::format(reason, std::forward<Args>(args)...))
    {}
};

} // namespace blackhole
