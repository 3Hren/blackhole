#pragma once

#include <functional>

#include <cppformat/format.h>

#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304
#include "blackhole/sandbox.hpp"
#endif

#include "blackhole/attributes.hpp"
#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {

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

template<typename... Args>
class args_t {
    typedef decltype(std::bind(&gcc::write_all<Args...>, std::placeholders::_1,
        // std::declval<string_view>().data(),
        std::placeholders::_2,
        std::declval<std::reference_wrapper<const Args>>()...
    )) storage_type;

    mutable storage_type storage;

public:
    mutable const char* format;

    constexpr
    args_t(const Args&... args) noexcept:
        // storage(std::bind(&gcc::write_all<Args...>, std::placeholders::_1, format.data(), std::cref(args)...))
        storage(std::bind(&gcc::write_all<Args...>, std::placeholders::_1, std::placeholders::_2, std::cref(args)...)),
        format(nullptr)
    {}

    constexpr
    auto bind(const char* format) const -> void {
        this->format = format;
    }

    constexpr
    auto operator()(writer_t& wr) const -> void {
        storage(wr, format);
    }
};

template<typename... Args>
auto formatted(const Args&... args) -> args_t<Args...> {
    return args_t<Args...>(args...);
}

typedef view_of<attributes_t>::type attribute_list;

/// Helper metafunction that deduces the last type from the given variadic pack.
template<class... Tail>
struct last_of;

template<class T, class U, class... Tail>
struct last_of<T, U, Tail...> {
    typedef typename last_of<U, Tail...>::type type;
};

template<class T>
struct last_of<T> {
    typedef T type;
};

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

template<typename... Args>
struct with_attributes : public std::is_same<typename last_of<Args...>::type, attribute_list> {};

template<template<typename...> class F, typename... Args>
struct without_tail {
    typedef typename detail::internal_t<F, detail::dummy_t<>, Args...>::type type;
};

}  // namespace detail

/// Logging facade wraps the underlying logger providing convenient format methods.
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

    template<typename... Args>
    auto Log(int severity,
             const string_view& format,
             const args_t<Args...>& args,
             const attribute_list& attributes) const -> void
    {
        args.bind(format.data());

        range_t range{attributes};
        inner().log(severity, format, range, std::cref(args));
    }

    template<typename... Args>
    auto Hog(int severity, const string_view& format, const Args&... args) const -> void {
        deduce(severity, format, args...);
    }

    /// Log a message with the given severity level.
    auto log(int severity, string_view format) const -> void;

    /// Log a message with the given severity level and further formatting using the given pattern
    /// and arguments.
    ///
    /// \overload
    /// \tparam T and Args... must meet the requirements of `StreamFormatted`.
    template<typename T, typename... Args>
    auto log(int severity, string_view format, const T& arg, const Args&... args) const -> void;

    /// Log a message with the given severity level and attributes.
    ///
    /// \overload
    auto log(int severity, const attribute_list& attributes, string_view format) const -> void;

    /// Log a message with the given severity level and attributes and further formatting using the
    /// given pattern and arguments.
    ///
    /// \overload
    /// \tparam T and Args... must meet the requirements of `StreamFormatted`.
    // TODO: Find a way to make the `attribute_list` argument last.
    template<typename T, typename... Args>
    auto log(int severity, const attribute_list& attributes, string_view format, const T& arg, const Args&... args) const -> void;

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

    template<typename... Args>
    inline
    auto deduce(int severity, const string_view& format, const Args&... args) const ->
        typename std::enable_if<!detail::with_attributes<Args...>::value>::type
    {
        const auto fn = std::bind(&gcc::write_all<Args...>, std::placeholders::_1, format.data(), std::cref(args)...);

        range_t range;
        inner().log(severity, format, range, std::cref(fn));
    }

    template<class... Args>
    struct applier_t {
        static
        auto apply(const wrapped_type& log,
                   int severity,
                   const string_view& format,
                   const Args&... args,
                   const attribute_list& attributes) -> void
        {
            const auto fn = std::bind(&gcc::write_all<Args...>, std::placeholders::_1, format.data(), std::cref(args)...);

            range_t range{attributes};
            log.log(severity, format, range, std::cref(fn));
        }
    };

    template<typename... Args>
    constexpr
    auto deduce(int severity, const string_view& format, const Args&... args) const ->
        typename std::enable_if<detail::with_attributes<Args...>::value>::type
    {
        detail::without_tail<applier_t, Args...>::type::apply(inner(), severity, format, args...);
    }
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
    const auto fn = std::bind(&gcc::write_all<T, Args...>, std::placeholders::_1, format.data(), std::cref(arg), std::cref(args)...);

    range_t range;
    inner().log(severity, format, range, std::cref(fn));
}

template<typename Logger>
inline
auto
logger_facade<Logger>::log(int severity, const attribute_list& attributes, string_view format) const -> void {
    range_t range{attributes};
    inner().log(severity, format, range);
}

template<typename Logger>
template<typename T, typename... Args>
inline
auto
logger_facade<Logger>::log(int severity, const attribute_list& attributes, string_view format, const T& arg, const Args&... args) const -> void {
    const auto fn = std::bind(&gcc::write_all<T, Args...>, std::placeholders::_1, format.data(), std::cref(arg), std::cref(args)...);

    range_t range{attributes};
    inner().log(severity, format, range, std::cref(fn));
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

}  // namespace blackhole
