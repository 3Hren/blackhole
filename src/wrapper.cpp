#include "blackhole/wrapper.hpp"

#include "blackhole/scoped.hpp"

namespace blackhole {

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

auto
wrapper_t::log(int severity, string_view pattern) -> void {
    attribute_pack pack{attributes()};
    inner.log(severity, pattern, pack);
}

auto
wrapper_t::log(int severity, string_view pattern, attribute_pack& pack) -> void {
    pack.push_back(attributes());
    inner.log(severity, pattern, pack);
}

auto
wrapper_t::log(int severity, string_view pattern, attribute_pack& pack, const supplier_t& supplier) -> void {
    pack.push_back(attributes());
    inner.log(severity, pattern, pack, supplier);
}

auto
wrapper_t::scoped(attributes_t attributes) -> scoped_t {
    return inner.scoped(std::move(attributes));
}

}  // namespace blackhole
