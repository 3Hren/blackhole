#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>

#include <boost/mpl/contains.hpp>
#include <boost/mpl/vector.hpp>

#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {
inline namespace v1 {

/// Trait that describes how to format user defined types provided as attributes.
template<typename T>
struct display_traits;

/// Represents a trait for mapping an owned types to their associated lightweight view types.
///
/// By default all types are transparently mapped to itself, but Blackhole provides some
/// specializations.
///
/// \warning it is undefined behavior to add specializations for this trait.
template<typename T>
struct view_of {
    typedef T type;
};

/// Forward.
class writer_t;

}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace attribute {

class value_t;
class view_t;

/// Represents an attribute value holder.
///
/// Attribute value is an algebraic data type that can be initialized with one of the following
/// predefined primitive types:
///     - none marker;
///     - boolean type (true or false);
///     - signed integer types up to 64-bit size;
///     - unsigned integer types up to 64-bit size;
///     - floating point type;
///     - owned string type;
///     - and a function type, which can fill the specified writer with the value lazily.
///
/// The underlying value can be obtained through `blackhole::attribute::get` function with providing
/// the desired result type. For example:
///     blackhole::attribute::value_t value(42);
///     const auto actual = blackhole::attribute::get<std::int64_t>(value);
///
///     assert(42 == actual);
///
/// As an alternative a visitor pattern is provided for enabling underlying value visitation. To
/// enable this feature, implement the `value_t::visitor_t` interface and provide an instance of
/// this implementation to the `apply` method.
class value_t {
public:
    /// Available types.
    typedef std::nullptr_t null_type;
    typedef bool           bool_type;
    typedef std::int64_t   sint64_type;
    typedef std::uint64_t  uint64_type;
    typedef double         double_type;
    typedef std::string    string_type;
    typedef std::function<auto(writer_t& writer) -> void> function_type;

    /// The type sequence of all available types.
    typedef boost::mpl::vector<
        null_type,
        bool_type,
        sint64_type,
        uint64_type,
        double_type,
        string_type,
        function_type
    > types;

    /// Visitor interface.
    class visitor_t {
    public:
        virtual ~visitor_t() = 0;
        virtual auto operator()(const null_type&) -> void = 0;
        virtual auto operator()(const bool_type&) -> void = 0;
        virtual auto operator()(const sint64_type&) -> void = 0;
        virtual auto operator()(const uint64_type&) -> void = 0;
        virtual auto operator()(const double_type&) -> void = 0;
        virtual auto operator()(const string_type&) -> void = 0;
        virtual auto operator()(const function_type&) -> void = 0;
    };

    struct inner_t;

private:
    typedef std::aligned_storage<sizeof(function_type) + sizeof(int)>::type storage_type;

    /// The underlying type.
    storage_type storage;

public:
    /// Constructs a null value containing tagged nullptr value.
    value_t();

    value_t(std::nullptr_t);

    /// Constructs a value initialized with the given boolean value.
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
    auto apply(visitor_t& visitor) const -> void;

    /// Returns the internal underlying value.
    auto inner() noexcept -> inner_t&;
    auto inner() const noexcept -> const inner_t&;

private:
    template<typename T>
    auto construct(T&& value) -> void;
};

/// Represents an attribute value holder view, containing only lightweight views of the actual
/// values.
///
/// If an owned value is small enough to keep its copy - this class does it, otherwise keeping only
/// view proxy values. For example for `std::string` values there is a lightweight mapping that
/// holds only two members: a pointer to constant char and a size.
///
/// The underlying value can also be obtained through `blackhole::attribute::get` function with
/// providing the desired result type. For example:
///     blackhole::attribute::view_t value("le vinegret");
///     const auto actual = blackhole::attribute::get<std::int64_t>(value);
///
///     assert("le vinegret" == actual);
///
class view_t {
public:
    /// Available types.
    typedef std::nullptr_t null_type;
    typedef bool           bool_type;
    typedef std::int64_t   sint64_type;
    typedef std::uint64_t  uint64_type;
    typedef double         double_type;
    typedef string_view    string_type;

    struct function_type {
        const void* value;
        std::reference_wrapper<auto(const void* value, writer_t& writer) -> void> fn;

        auto operator()(writer_t& wr) const -> void {
            fn(value, wr);
        }

        auto operator==(const function_type& other) const noexcept -> bool {
            return value == other.value && fn.get() == other.fn.get();
        }
    };

    /// The type sequence of all available types.
    typedef boost::mpl::vector<
        null_type,
        bool_type,
        sint64_type,
        uint64_type,
        double_type,
        string_type,
        function_type
    > types;

    /// Visitor interface.
    class visitor_t {
    public:
        virtual ~visitor_t() = 0;
        virtual auto operator()(const null_type&) -> void = 0;
        virtual auto operator()(const bool_type&) -> void = 0;
        virtual auto operator()(const sint64_type&) -> void = 0;
        virtual auto operator()(const uint64_type&) -> void = 0;
        virtual auto operator()(const double_type&) -> void = 0;
        virtual auto operator()(const string_type&) -> void = 0;
        virtual auto operator()(const function_type&) -> void = 0;
    };

    struct inner_t;

private:
    typedef std::aligned_storage<2 * sizeof(void*) + sizeof(int)>::type storage_type;

    /// The underlying type.
    storage_type storage;

public:
    /// Constructs a null value view containing tagged nullptr value.
    view_t();

    view_t(std::nullptr_t);

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
    view_t(float value);
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

    /// Constructs a value view from a custom type that implements `display_traits` trait.
    ///
    /// \sa display_traits for more information.
    template<typename T>
    view_t(const T& value, decltype(&display_traits<T>::apply)* = nullptr) {
        construct(function_type{static_cast<const void*>(&value), std::ref(display<T>)});
    }

    view_t(const view_t& other) = default;
    view_t(view_t&& other) = default;

    ~view_t() = default;

    auto operator=(const view_t& other) -> view_t& = default;
    auto operator=(view_t&& other) -> view_t& = default;

    /// Applies the given visitor to perform pattern matching.
    auto apply(visitor_t& visitor) const -> void;

    auto operator==(const view_t& other) const -> bool;
    auto operator!=(const view_t& other) const -> bool;

    /// Returns the internal underlying value.
    auto inner() noexcept -> inner_t&;
    auto inner() const noexcept -> const inner_t&;

private:
    template<typename T>
    auto construct(T&& value) -> void;

    template<typename T>
    static auto display(const void* value, writer_t& wr) -> void {
        display_traits<T>::apply(*static_cast<const T*>(value), wr);
    }
};

/// Retrieves a value of a specified, but yet restricted type, from a given attribute value.
///
/// \throws std::bad_cast if the content is not of the specified type T.
template<typename T>
auto get(const value_t& value) ->
    typename std::enable_if<boost::mpl::contains<value_t::types, T>::value, const T&>::type;

/// Retrieves a value of a specified, but yet restricted type, from a given attribute value view.
///
/// \throws std::bad_cast if the content is not of the specified type T.
template<typename T>
auto get(const view_t& value) ->
    typename std::enable_if<boost::mpl::contains<view_t::types, T>::value, const T&>::type;

}  // namespace attribute
}  // namespace v1
}  // namespace blackhole
