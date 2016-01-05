#include "blackhole/attribute.hpp"
#include "blackhole/scoped.hpp"

#include <boost/thread/tss.hpp>

namespace blackhole {

using attribute::view_t;

namespace {

// TODO: Move somewhere near `view_of`.
auto transform(const attributes_t& source) -> attribute_list {
    attribute_list result;
    for (const auto& attribute : source) {
        result.emplace_back(attribute);
    }

    return result;
}

}  // namespace

scoped_t::scoped_t(logger_t& logger, attributes_t attributes):
    logger_t::scoped_t(logger),
    storage(std::move(attributes)),
    list(transform(storage))
{}

auto scoped_t::attributes() const -> const attribute_list& {
    return list;
}

}  // namespace blackhole
