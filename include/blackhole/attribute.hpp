#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

#include <boost/mpl/vector.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant.hpp>

#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {

template<typename T>
struct view_of {
    typedef T type;
};

template<>
struct view_of<std::string> {
    typedef string_view type;
};

}  // namespace blackhole

namespace blackhole {
namespace attribute {

class bad_get;
class value_t;
class view_t;

class bad_get : public std::logic_error {
public:
    bad_get() : std::logic_error("bad get") {}
};

/// Represents an attribute value holder.
class value_t {
public:
    typedef boost::mpl::vector<
        // std::nullptr_t
        // bool,
        std::int64_t,
        // std::uint64_t,
        double,
        std::string
    >::type types;

    typedef boost::make_variant_over<types>::type type;

// private:
    /// Underlying type.
    type inner;

public:
    value_t(int val): inner(static_cast<std::int64_t>(val)) {}
    value_t(double val): inner(val) {}
    value_t(std::string val): inner(std::move(val)) {}

    /// Visitor.
    template<typename Visitor>
    auto apply(Visitor&& visitor) const -> typename Visitor::result_type {
        return boost::apply_visitor(std::forward<Visitor>(visitor), inner);
    }
};

class view_t {
public:
    /// Available types.
    typedef std::nullptr_t null_type;
    typedef bool           bool_type;
    typedef std::int64_t   sint64_type;
    typedef std::uint64_t  uint64_type;
    typedef double         double_type;
    typedef string_view    string_type;

    typedef boost::mpl::vector<
        sint64_type,
        double_type,
        string_type
    > types;

    class visitor_t {
    public:
        virtual ~visitor_t() = 0;
        virtual auto operator()(const null_type&) -> void = 0;
        virtual auto operator()(const bool_type&) -> void = 0;
        virtual auto operator()(const sint64_type&) -> void = 0;
        virtual auto operator()(const uint64_type&) -> void = 0;
        virtual auto operator()(const double_type&) -> void = 0;
        virtual auto operator()(const string_type&) -> void = 0;
    };

    struct inner_t;

private:
    typedef std::aligned_storage<2 * sizeof(void*) + sizeof(int)>::type storage_type;

    /// The underlying type.
    storage_type storage;

public:
    /// Constructs a null view value.
    view_t();

    /// Conversion constructors.
    view_t(int value);
    // view_t(char value);
    view_t(long value);
    view_t(long long value);

    // view_t(unsigned int value);
    // view_t(unsigned char value);
    // view_t(unsigned long value);
    // view_t(unsigned long long value);

    // view_t(float value);
    view_t(double value);

    view_t(const string_type& value);

    /// Conversion constructor from an owned type.
    view_t(const value_t& value);

    view_t(const view_t& other) = default;
    view_t(view_t&& other) = default;

    ~view_t() = default;

    auto operator=(const view_t& other) -> view_t& = default;
    auto operator=(view_t&& other) -> view_t& = default;

    /// Visitor.
    auto apply(const visitor_t& visitor) const -> void;

    auto operator==(const view_t& other) const -> bool;
    auto operator!=(const view_t& other) const -> bool;

    auto inner() noexcept -> inner_t&;
    auto inner() const noexcept -> const inner_t&;
};

/// Retrieves a value of a specified, but yet restricted type, from a given attribute value view.
template<typename T>
auto get(const view_t& value) -> const T&;

template<> auto get<view_t::null_type>(const view_t& value) -> const view_t::null_type&;
template<> auto get<view_t::bool_type>(const view_t& value) -> const view_t::bool_type&;
template<> auto get<view_t::sint64_type>(const view_t& value) -> const view_t::sint64_type&;
template<> auto get<view_t::uint64_type>(const view_t& value) -> const view_t::uint64_type&;
template<> auto get<view_t::double_type>(const view_t& value) -> const view_t::double_type&;
template<> auto get<view_t::string_type>(const view_t& value) -> const view_t::string_type&;

}  // namespace attribute
}  // namespace blackhole
