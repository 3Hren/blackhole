#include "blackhole/scoped/keeper.hpp"

namespace blackhole {
inline namespace v1 {
namespace scoped {

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

keeper_t::keeper_t(logger_t& logger, attributes_t attributes):
    scoped_t(logger),
    storage(std::move(attributes)),
    list(transform(storage))
{}

auto keeper_t::attributes() const -> const attribute_list& {
    return list;
}

}  // namespace scoped
}  // namespace v1
}  // namespace blackhole
