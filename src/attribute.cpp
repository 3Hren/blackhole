#include "blackhole/attribute.hpp"

#include <boost/type_traits/remove_cv.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>

#include "blackhole/detail/attribute.hpp"

namespace blackhole {
inline namespace v1 {
namespace attribute {
namespace {

auto call(const void* value, writer_t& wr) -> void {
    auto fn = *static_cast<const std::function<auto(writer_t&) -> void>*>(value);
    fn(wr);
}

struct into_view {
    typedef view_t::inner_t::type result_type;

    auto operator()(const value_t::function_type& value) const -> result_type {
        return view_t::function_type{static_cast<const void*>(&value), std::ref(call)};
    }

    template<typename T>
    auto operator()(const T& value) const -> result_type {
        return value;
    }
};

template<typename Value>
struct from {
    typedef void result_type;

    typename Value::visitor_t& visitor;

    template<typename T>
    auto operator()(const T& value) const -> void {
        visitor(value);
    }
};

}  // namespace

static_assert(sizeof(value_t::inner_t) <= sizeof(value_t), "padding or alignment violation");
static_assert(sizeof(view_t::inner_t) <= sizeof(view_t), "padding or alignment violation");

value_t::visitor_t::~visitor_t() = default;

value_t::value_t() {
    construct(nullptr);
}

value_t::value_t(bool value) {
    construct(value);
}

value_t::value_t(char value) {
    construct(static_cast<std::int64_t>(value));
}

value_t::value_t(short value) {
    construct(static_cast<std::int64_t>(value));
}

value_t::value_t(int value) {
    construct(static_cast<std::int64_t>(value));
}

value_t::value_t(long value) {
    construct(static_cast<std::int64_t>(value));
}

value_t::value_t(long long value) {
    construct(static_cast<std::int64_t>(value));
}

value_t::value_t(unsigned char value) {
    construct(static_cast<std::uint64_t>(value));
}

value_t::value_t(unsigned short value) {
    construct(static_cast<std::uint64_t>(value));
}

value_t::value_t(unsigned int value) {
    construct(static_cast<std::uint64_t>(value));
}

value_t::value_t(unsigned long value) {
    construct(static_cast<std::uint64_t>(value));
}

value_t::value_t(unsigned long long value) {
    construct(static_cast<std::uint64_t>(value));
}

value_t::value_t(double value) {
    construct(value);
}

value_t::value_t(std::string value) {
    construct(std::move(value));
}

value_t::value_t(const value_t& other) {
    construct(other.inner().value);
}

value_t::value_t(value_t&& other) {
    construct(std::move(other.inner().value));
}

auto value_t::operator=(const value_t& other) -> value_t& {
    if (this != &other) {
        inner() = other.inner();
    }

    return *this;
}

auto value_t::operator=(value_t&& other) -> value_t& {
    if (this != &other) {
        inner() = std::move(other.inner());
    }

    return *this;
}

value_t::~value_t() {
    inner().~inner_t();
}

auto value_t::apply(visitor_t& visitor) const -> void {
    boost::apply_visitor(from<value_t>{visitor}, inner().value);
}

auto value_t::inner() noexcept -> inner_t& {
    return reinterpret_cast<inner_t&>(storage);
}

auto value_t::inner() const noexcept -> const inner_t& {
    return reinterpret_cast<const inner_t&>(storage);
}

template<typename T>
auto value_t::construct(T&& value) -> void {
    new(static_cast<void*>(&storage)) inner_t{std::forward<T>(value)};
}

template<typename T>
auto get(const value_t& value) ->
    typename std::enable_if<boost::mpl::contains<value_t::types, T>::value, const T&>::type
{
    if (auto result = boost::get<T>(&value.inner().value)) {
        return *result;
    } else {
        throw std::bad_cast();
    }
}

template auto value_t::construct<std::string>(std::string&& value) -> void;

template auto get<value_t::null_type>(const value_t& value) -> const value_t::null_type&;
template auto get<value_t::bool_type>(const value_t& value) -> const value_t::bool_type&;
template auto get<value_t::sint64_type>(const value_t& value) -> const value_t::sint64_type&;
template auto get<value_t::uint64_type>(const value_t& value) -> const value_t::uint64_type&;
template auto get<value_t::double_type>(const value_t& value) -> const value_t::double_type&;
template auto get<value_t::string_type>(const value_t& value) -> const value_t::string_type&;

view_t::visitor_t::~visitor_t() = default;

view_t::view_t() {
    construct(nullptr);
}

view_t::view_t(std::nullptr_t) {
    construct(nullptr);
}

view_t::view_t(bool value) {
    construct(value);
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

view_t::view_t(float value) {
    construct(static_cast<double>(value));
}

view_t::view_t(double value) {
    construct(value);
}

view_t::view_t(const string_type& value) {
    construct(value);
}

view_t::view_t(const std::string& value) {
    construct(value);
}

view_t::view_t(const value_t& value) {
    construct(boost::apply_visitor(into_view(), value.inner().value));
}

auto view_t::operator==(const view_t& other) const -> bool {
    return inner().value == other.inner().value;
}

auto view_t::apply(visitor_t& visitor) const -> void {
    boost::apply_visitor(from<view_t>{visitor}, inner().value);
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

template auto view_t::construct<string_view>(string_view&& value) -> void;
template auto view_t::construct<view_t::function_type>(view_t::function_type&& value) -> void;

template auto get<view_t::null_type>(const view_t& value) -> const view_t::null_type&;
template auto get<view_t::bool_type>(const view_t& value) -> const view_t::bool_type&;
template auto get<view_t::sint64_type>(const view_t& value) -> const view_t::sint64_type&;
template auto get<view_t::uint64_type>(const view_t& value) -> const view_t::uint64_type&;
template auto get<view_t::double_type>(const view_t& value) -> const view_t::double_type&;
template auto get<view_t::string_type>(const view_t& value) -> const view_t::string_type&;
template auto get<view_t::function_type>(const view_t& value) -> const view_t::function_type&;

}  // namespace attribute
}  // namespace v1
}  // namespace blackhole
