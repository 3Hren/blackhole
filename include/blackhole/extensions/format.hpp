#pragma once

#include <functional>

#include <cppformat/format.h>

#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304
#include "blackhole/sandbox.hpp"
#endif

#include "blackhole/attributes.hpp"
#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {

namespace ph = std::placeholders;
namespace cppformat = fmt;

class writer_t {
public:
    cppformat::MemoryWriter inner;

    template<typename... Args>
    inline auto write(const Args&... args) -> void {
        inner.write(args...);
    }
};

namespace gcc {

/// Workaround for GCC versions up to 4.9, which fails to expand variadic pack inside lambdas.
template<class... Args>
inline void write_all(writer_t& wr, const char* pattern, const Args&... args) {
    wr.write(pattern, args...);
}

}  // namespace gcc

typedef view_of<attributes_t>::type attribute_list;

namespace detail {

/// Helper metafunction that deduces the last type from the given variadic pack.
///
/// For example:
///     last_of<int>::type         => int.
///     last_of<int, double>::type => double.
template<typename T, typename... Tail>
struct last_of;

template<typename T, typename U, typename... Tail>
struct last_of<T, U, Tail...> {
    typedef typename last_of<U, Tail...>::type type;
};

template<typename T>
struct last_of<T> {
    typedef T type;
};

/// Helper metafunction that determines whether the last parameter from given variadic pack is an
/// attributes list.
///
/// For example:
///     with_attributes<int, double>::type                 => std::false_type.
///     with_attributes<int, double, attribute_list>::type => std::true_type.
template<typename... Args>
struct with_attributes : public std::is_same<typename last_of<Args...>::type, attribute_list> {};

}  // namespace detail

/// Logging facade wraps the underlying logger and provides convenient formatting methods.
///
/// \tparam Logger must meet the requirements of `Logger`.
template<typename Logger>
class logger_facade {
public:
    typedef Logger wrapped_type;

private:
    std::reference_wrapper<const wrapped_type> wrapped;

public:
    constexpr explicit logger_facade(const wrapped_type& wrapped) noexcept:
        wrapped(wrapped)
    {}

    /// Log a message with the given severity level.
    auto log(int severity, string_view format) const -> void;

    /// Log a message with the given severity level and attributes.
    ///
    /// \overload
    auto log(int severity, string_view format, const attribute_list& attributes) const -> void;

    /// Log a message with the given severity level and further formatting using the given pattern
    /// and arguments with optional attributes list as a last parameter.
    ///
    /// \overload
    /// \tparam T and Args... must meet the requirements of `StreamFormatted`.
    /// \note the last parameter of variadic `Args...` pack can be an `attribute_list`.
    template<typename T, typename... Args>
    auto log(int severity, string_view format, const T& arg, const Args&... args) const -> void;

#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304
    /// Log a message with the given severity level and further formatting using the given pattern
    /// and arguments.
    ///
    /// This is more fast alternative comparing with default formatting, because the given pattern
    /// string is parsed into literals during compile time.
    ///
    /// \overload
    /// \tparam T and Args... must meet the requirements of `StreamFormatted`.
    template<std::size_t N, typename T, typename... Args>
    auto log(int severity, const detail::formatter<N>& formatter, const T& arg, const Args&... args) const -> void;

    /// Log a message with the given severity level and attributes and further formatting using the
    /// given pattern and arguments.
    ///
    /// This is more fast alternative comparing with default formatting, because the given pattern
    /// string is parsed into literals during compile time.
    ///
    /// \overload
    /// \tparam T and Args... must meet the requirements of `StreamFormatted`.
    template<std::size_t N, typename T, typename... Args>
    auto log(int severity, const attribute_list& attributes, const detail::formatter<N>& formatter, const T& arg, const Args&... args) const -> void;
#endif

private:
    constexpr auto inner() const noexcept -> const wrapped_type& {
        return wrapped.get();
    }

    /// Selects the proper method overload when using variadic pack interface.
    ///
    /// \overload for variadic pack without attribute list as the last argument.
    template<typename... Args>
    inline
    auto select(int severity, const string_view& format, const Args&... args) const ->
        typename std::enable_if<!detail::with_attributes<Args...>::value>::type;

    /// Selects the proper method overload when using variadic pack interface.
    ///
    /// \overload for variadic pack with attribute list as the last argument.
    template<typename... Args>
    constexpr
    auto select(int severity, const string_view& format, const Args&... args) const ->
        typename std::enable_if<detail::with_attributes<Args...>::value>::type;
};

template<typename Logger>
inline
auto
logger_facade<Logger>::log(int severity, string_view format) const -> void {
    range_t range;
    inner().log(severity, format, range);
}

template<typename Logger>
template<typename T, typename... Args>
inline
auto
logger_facade<Logger>::log(int severity, string_view format, const T& arg, const Args&... args) const -> void {
    select(severity, format, arg, args...);
}

template<typename Logger>
inline
auto
logger_facade<Logger>::log(int severity, string_view format, const attribute_list& attributes) const -> void {
    range_t range{attributes};
    inner().log(severity, format, range);
}

#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304

template<typename Logger>
template<std::size_t N, typename T, typename... Args>
inline
auto
logger_facade<Logger>::log(int severity, const detail::formatter<N>& formatter, const T& arg, const Args&... args) const -> void {
    const auto fn = [&](writer_t& wr) {
        formatter.format(wr.inner, arg, args...);
    };

    range_t range;
    inner().log(severity, "", range, std::cref(fn));
}

#endif

namespace detail {

template<typename... Args>
struct dummy_t {};

template<template<typename...> class F, typename T, typename... Args>
struct internal_t;

template<template<typename...> class F, typename... Args, typename T, typename... Tail>
struct internal_t<F, dummy_t<Args...>, T, Tail...> {
    typedef typename internal_t<F, dummy_t<Args..., T>, Tail...>::type type;
};

template<template<typename...> class F, typename... Args, typename Tail, typename Last>
struct internal_t<F, dummy_t<Args...>, Tail, Last> {
    typedef F<Args..., Tail> type;
};

template<template<typename...> class F, typename... Args>
struct without_tail {
    typedef typename detail::internal_t<F, detail::dummy_t<>, Args...>::type type;
};

template<typename... Args>
struct select_t {
    template<typename Logger>
    static
    auto apply(const Logger& log,
               int severity,
               const string_view& format,
               const Args&... args,
               const attribute_list& attributes) -> void
    {
        const auto fn = std::bind(&gcc::write_all<Args...>, ph::_1, format.data(), std::cref(args)...);

        range_t range{attributes};
        log.log(severity, format, range, std::cref(fn));
    }
};

}  // namespace detail

template<typename Logger>
template<typename... Args>
inline
auto
logger_facade<Logger>::select(int severity, const string_view& format, const Args&... args) const ->
    typename std::enable_if<!detail::with_attributes<Args...>::value>::type
{
    const auto fn = std::bind(&gcc::write_all<Args...>, ph::_1, format.data(), std::cref(args)...);

    range_t range;
    inner().log(severity, format, range, std::cref(fn));
}

template<typename Logger>
template<typename... Args>
constexpr
auto
logger_facade<Logger>::select(int severity, const string_view& format, const Args&... args) const ->
    typename std::enable_if<detail::with_attributes<Args...>::value>::type
{
    detail::without_tail<detail::select_t, Args...>::type::apply(inner(), severity, format, args...);
}

}  // namespace blackhole
