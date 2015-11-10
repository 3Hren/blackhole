#pragma once

#include "blackhole/logger.hpp"

namespace blackhole {

class wrapper_t : public logger_t {
    logger_t& inner;
    attributes_t storage;
    view_of<attributes_t>::type attributes_view;

public:
    wrapper_t(logger_t& log, attributes_t attributes);

    auto attributes() const noexcept -> const view_of<attributes_t>::type& {
        return attributes_view;
    }

    auto log(int severity, string_view message) const -> void;
    auto log(int severity, string_view message, attribute_pack& range) const -> void;
    auto log(int severity, string_view message, attribute_pack& range, const format_t& fn) const -> void;
};

}  // namespace blackhole
