#pragma once

#include "blackhole/detail/datetime.hpp"

namespace blackhole {

namespace mapping {

enum class timezone_t {
    gmtime,
    localtime
};

template<timezone_t>
class timepicker_t;

template<>
class timepicker_t<timezone_t::localtime> {
public:
    static inline std::tm pick(const timeval& time) {
        std::tm tm;
        localtime_r(&time.tv_sec, &tm);
        return tm;
    }
};

template<>
class timepicker_t<timezone_t::gmtime> {
public:
    static inline std::tm pick(const timeval& time) {
        std::tm tm;
        gmtime_r(&time.tv_sec, &tm);
        return tm;
    }
};

template<class TimePicker = timepicker_t<timezone_t::gmtime>>
struct datetime_formatter_action_t {
    typedef TimePicker picker_type;

    aux::datetime::generator_t generator;

    datetime_formatter_action_t(const std::string& format) :
        generator(aux::datetime::generator_factory_t::make(format))
    {}

    void
    operator()(aux::attachable_ostringstream& stream, const timeval& value) const {
        generator(stream, picker_type::pick(value), value.tv_usec);
    }
};

} // namespace mapping

} // namespace blackhole
