#include "blackhole/wrapper.hpp"

namespace blackhole {

wrapper_t::wrapper_t(logger_t& log, attributes_t attributes):
    inner(log),
    storage(std::move(attributes))
{
    for (const auto& attribute : storage) {
        attributes_view.emplace_back(attribute);
    }
}

auto
wrapper_t::log(int severity, string_view message) const -> void {
    attribute_pack range{attributes()};
    inner.log(severity, message, range);
}

auto
wrapper_t::log(int severity, string_view message, attribute_pack& range) const -> void {
    range.push_back(attributes());
    inner.log(severity, message, range);
}

auto
wrapper_t::log(int severity, string_view message, attribute_pack& range, const format_t& fn) const -> void {
    range.push_back(attributes());
    inner.log(severity, message, range, fn);
}

}  // namespace blackhole
