#include "blackhole/wrapper.hpp"

namespace blackhole {
inline namespace v1 {

using attribute::view_t;

wrapper_t::wrapper_t(logger_t& log, attributes_t attributes):
    inner(log),
    storage(std::move(attributes))
{
    // TODO: Replace somewhere near `view_of`.
    for (const auto& attribute : storage) {
        attributes_view.emplace_back(attribute);
    }
}

auto wrapper_t::log(severity_t severity, const message_t& message) -> void {
    attribute_pack pack{attributes()};
    inner.log(severity, message, pack);
}

auto wrapper_t::log(severity_t severity, const message_t& message, attribute_pack& pack) -> void {
    pack.push_back(attributes());
    inner.log(severity, message, pack);
}

auto wrapper_t::log(severity_t severity, const lazy_message_t& message, attribute_pack& pack) -> void {
    pack.push_back(attributes());
    inner.log(severity, message, pack);
}

auto wrapper_t::manager() -> scope::manager_t& {
    return inner.manager();
}

}  // namespace v1
}  // namespace blackhole
