#include "blackhole/wrapper.hpp"

namespace blackhole {

wrapper_t::wrapper_t(logger_t& log, attributes_w_t owned_):
    inner(log),
    owned(std::move(owned_))
{
    for (const auto& a : owned) {
        attributes.push_back(std::make_pair(string_view(a.first.data(), a.first.size()), a.second));
    }
}

auto
wrapper_t::execute(int severity, string_view message, range_t& range) const -> void {}

auto
wrapper_t::execute(int severity, string_view message, range_t& range, const format_t& fn) const -> void {
    range.push_back(attributes);
    inner.execute(severity, message, range, fn);
}

}  // namespace blackhole
