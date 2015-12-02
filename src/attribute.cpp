#include "blackhole/attribute.hpp"

#include <boost/variant/get.hpp>

#include "blackhole/detail/attribute.hpp"

namespace blackhole {
namespace attribute {
namespace {

struct into_view {
    typedef view_t::inner_t::type result_type;

    template<typename T>
    auto operator()(const T& value) const -> result_type {
        return value;
    }
};

}  // namespace

static_assert(sizeof(view_t::inner_t) <= sizeof(view_t), "padding or alignment violation");

view_t::view_t(int value) {
    new(static_cast<void*>(&storage)) inner_t{static_cast<std::int64_t>(value)};
}

view_t::view_t(long value) {
    new(static_cast<void*>(&storage)) inner_t{static_cast<std::int64_t>(value)};
}

view_t::view_t(double value) {
    new(static_cast<void*>(&storage)) inner_t{value};
}

view_t::view_t(const string_type& value) {
    new(static_cast<void*>(&storage)) inner_t{value};
}

view_t::view_t(const value_t& value) {
    new(static_cast<void*>(&storage)) inner_t{boost::apply_visitor(into_view(), value.inner)};
}

auto view_t::operator==(const view_t& other) const -> bool {
    return inner().value == other.inner().value;
}

auto view_t::inner() noexcept -> inner_t& {
    return reinterpret_cast<inner_t&>(storage);
}

auto view_t::inner() const noexcept -> const inner_t& {
    return reinterpret_cast<const inner_t&>(storage);
}

template<>
auto get<std::int64_t>(const view_t& value) -> const std::int64_t& {
    if (auto result = boost::get<std::int64_t>(&value.inner().value)) {
        return *result;
    } else {
        throw bad_get();
    }
}

}  // namespace attribute
}  // namespace blackhole
