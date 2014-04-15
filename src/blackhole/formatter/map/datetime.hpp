#pragma once

#include "blackhole/detail/datetime.hpp"

namespace blackhole {

namespace mapping {

struct datetime_formatter_action_t {
    aux::datetime::generator_t generator;

    datetime_formatter_action_t(const std::string& format) :
        generator(aux::datetime::generator_factory_t::make(format))
    {}

    void operator()(aux::attachable_ostringstream& stream, const timeval& value) const {
        std::tm tm;
        gmtime_r(&value.tv_sec, &tm);
        generator(stream, tm, value.tv_usec);
    }
};

} // namespace mapping

} // namespace blackhole
