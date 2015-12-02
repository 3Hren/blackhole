#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

#include <boost/mpl/contains.hpp>
#include <boost/mpl/vector.hpp>

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

class value_t;
class view_t;

/// Represents an attribute value holder.
class value_t {
public:
    /// Available types.
    typedef std::nullptr_t null_type;
    typedef bool           bool_type;
    typedef std::int64_t   sint64_type;
    typedef std::uint64_t  uint64_type;
    typedef double         double_type;
    typedef std::string    string_type;

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
    typedef std::aligned_storage<sizeof(std::string) + sizeof(int)>::type storage_type;

    /// The underlying type.
    storage_type storage;

public:
    value_t();

    value_t(bool value);

    /// Constructs a value initialized with the given signed integer.
    value_t(char value);
    value_t(short value);
    value_t(int value);
    value_t(long value);
    value_t(long long value);

    /// Constructs a value initialized with the given unsigned integer.
    value_t(unsigned char value);
    value_t(unsigned short value);
    value_t(unsigned int value);
    value_t(unsigned long value);
    value_t(unsigned long long value);

    value_t(double value);

    /// Constructs a value from the given string literal not including the terminating null
    /// character.
    ///
    /// \note this overload is required to prevent implicit conversion literal values to bool.
    template<std::size_t N>
    value_t(const char(&value)[N]) { construct(string_type{value, N - 1}); }

    value_t(std::string value);

    ~value_t();

    value_t(const value_t& other);
    value_t(value_t&& other);

    auto operator=(const value_t& other) -> value_t&;
    auto operator=(value_t&& other) -> value_t&;

    /// Applies the given visitor to perform pattern matching.
    auto apply(const visitor_t& visitor) const -> void;

    /// Returns the internal underlying value.
    auto inner() noexcept -> inner_t&;
    auto inner() const noexcept -> const inner_t&;

private:
    template<typename T>
    auto construct(T&& value) -> void;
};

/// Represents an attribute value holder view, containing only lightweight references to the actual
/// values.
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
    view_t(const char(&value)[N]) { construct(string_type{value, N - 1}); }

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

/// Retrieves a value of a specified, but yet restricted type, from a given attribute value.
template<typename T>
auto get(const value_t& value) ->
    typename std::enable_if<boost::mpl::contains<value_t::types, T>::value, const T&>::type;

/// Retrieves a value of a specified, but yet restricted type, from a given attribute value view.
template<typename T>
auto get(const view_t& value) ->
    typename std::enable_if<boost::mpl::contains<view_t::types, T>::value, const T&>::type;

}  // namespace attribute
}  // namespace blackhole
