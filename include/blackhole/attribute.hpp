#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

#include <boost/mpl/contains.hpp>
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
        null_type,
        bool_type,
        sint64_type,
        uint64_type,
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
    /// Constructs a null value view containing tagged nullptr value.
    view_t();

    /// Constructs a value view initialized with the given boolean value.
    view_t(bool value);

    /// Constructs a value view initialized with the given signed integer.
    view_t(char value);
    view_t(short value);
    view_t(int value);
    view_t(long value);
    view_t(long long value);

    /// Constructs a value view initialized with the given unsigned integer.
    view_t(unsigned char value);
    view_t(unsigned short value);
    view_t(unsigned int value);
    view_t(unsigned long value);
    view_t(unsigned long long value);

    /// Constructs a value view from the given floating point value.
    view_t(double value);

    /// Constructs a value view from the given string literal not including the terminating null
    /// character.
    ///
    /// \note this overload is required to prevent implicit conversion literal values to bool.
    template<std::size_t N>
    view_t(const char(&value)[N]) { construct(string_view{value, N - 1}); }

    /// Constructs a value view from the given string view.
    view_t(const string_type& value);

    /// Constructs a value view from the given string.
    view_t(const std::string& value);

    /// Constructs a value view from the given owned attribute value.
    view_t(const value_t& value);

    view_t(const view_t& other) = default;
    view_t(view_t&& other) = default;

    ~view_t() = default;

    auto operator=(const view_t& other) -> view_t& = default;
    auto operator=(view_t&& other) -> view_t& = default;

    /// Applies the given visitor to perform pattern matching.
    auto apply(const visitor_t& visitor) const -> void;

    auto operator==(const view_t& other) const -> bool;
    auto operator!=(const view_t& other) const -> bool;

    /// Returns the internal underlying value.
    auto inner() noexcept -> inner_t&;
    auto inner() const noexcept -> const inner_t&;

private:
    template<typename T>
    auto construct(T&& value) -> void;
};

/// Retrieves a value of a specified, but yet restricted type, from a given attribute value view.
template<typename T>
auto get(const view_t& value) ->
    typename std::enable_if<boost::mpl::contains<view_t::types, T>::value, const T&>::type;

}  // namespace attribute
}  // namespace blackhole
