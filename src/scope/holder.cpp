#include "blackhole/scope/holder.hpp"

namespace blackhole {
inline namespace v1 {
namespace scope {

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

holder_t::holder_t(logger_t& logger, attributes_t attributes):
    watcher_t(logger),
    storage(std::move(attributes)),
    list(transform(storage))
{}

auto holder_t::attributes() const -> const attribute_list& {
    return list;
}

}  // namespace scope
}  // namespace v1
}  // namespace blackhole
