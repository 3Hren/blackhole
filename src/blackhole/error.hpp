#pragma once

#include <stdexcept>

#include "blackhole/config.hpp"

#include "utils/format.hpp"

BLACKHOLE_BEG_NS

class error_t : public std::runtime_error {
public:
    template<typename... Args>
    error_t(const std::string& reason, Args&&... args) :
        std::runtime_error(utils::format(reason, std::forward<Args>(args)...))
    {}
};

BLACKHOLE_END_NS
