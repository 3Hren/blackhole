#pragma once

#include "blackhole/logger.hpp"

namespace blackhole {

class wrapper_t : public logger_t {
    logger_t& inner;
    attributes_t attributes;
    attributes_w_t owned;

public:
    wrapper_t(logger_t& log, attributes_w_t owned);

    auto log(int severity, string_view message) const -> void;
    auto log(int severity, string_view message, range_t& range) const -> void;
    auto log(int severity, string_view message, range_t& range, const format_t& fn) const -> void;
};

// TODO: auto make_wrapper(attributes_w_t attributes) -> std::unique_ptr<logger_t>;

}  // namespace blackhole
