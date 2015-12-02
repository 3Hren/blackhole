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

view_t::view_t() {
    construct(nullptr);
}

view_t::view_t(char value) {
    construct(static_cast<std::int64_t>(value));
}

view_t::view_t(short value) {
    construct(static_cast<std::int64_t>(value));
}

view_t::view_t(int value) {
    construct(static_cast<std::int64_t>(value));
}

view_t::view_t(long value) {
    construct(static_cast<std::int64_t>(value));
}

view_t::view_t(long long value) {
    construct(static_cast<std::int64_t>(value));
}

view_t::view_t(unsigned char value) {
    construct(static_cast<std::uint64_t>(value));
}

view_t::view_t(unsigned short value) {
    construct(static_cast<std::uint64_t>(value));
}

view_t::view_t(unsigned int value) {
    construct(static_cast<std::uint64_t>(value));
}

view_t::view_t(unsigned long value) {
    construct(static_cast<std::uint64_t>(value));
}

view_t::view_t(unsigned long long value) {
    construct(static_cast<std::uint64_t>(value));
}

view_t::view_t(double value) {
    construct(value);
}

view_t::view_t(const string_type& value) {
    construct(value);
}

auto view_t::from(const value_t& value) -> view_t {
    view_t view(0);
    view.construct(boost::apply_visitor(into_view(), value.inner));
    return view;
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

template<typename T>
auto view_t::construct(T&& value) -> void {
    new(static_cast<void*>(&storage)) inner_t{std::forward<T>(value)};
}

template<typename T>
auto get(const view_t& value) ->
    typename std::enable_if<boost::mpl::contains<view_t::types, T>::value, const T&>::type
{
    if (auto result = boost::get<T>(&value.inner().value)) {
        return *result;
    } else {
        throw std::bad_cast();
    }
}

template auto get<view_t::null_type>(const view_t& value) -> const view_t::null_type&;
// template auto get<view_t::bool_type>(const view_t& value) -> const view_t::bool_type&;
template auto get<view_t::sint64_type>(const view_t& value) -> const view_t::sint64_type&;
template auto get<view_t::uint64_type>(const view_t& value) -> const view_t::uint64_type&;
template auto get<view_t::double_type>(const view_t& value) -> const view_t::double_type&;
template auto get<view_t::string_type>(const view_t& value) -> const view_t::string_type&;

}  // namespace attribute
}  // namespace blackhole
