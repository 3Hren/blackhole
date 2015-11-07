#include "blackhole/wrapper.hpp"

namespace blackhole {

wrapper_t::wrapper_t(logger_t& log, attributes_w_t owned_):
    inner(log),
    owned(std::move(owned_))
{
    for (const auto& attribute : owned) {
        attributes_.emplace_back(attribute);
    }
}

auto
wrapper_t::log(int severity, string_view message) const -> void {
    range_t range{attributes()};
    inner.log(severity, message, range);
}

auto
wrapper_t::log(int severity, string_view message, range_t& range) const -> void {
    range.push_back(attributes());
    inner.log(severity, message, range);
}

auto
wrapper_t::log(int severity, string_view message, range_t& range, const format_t& fn) const -> void {
    range.push_back(attributes());
    inner.log(severity, message, range, fn);
}

}  // namespace blackhole
