#pragma once

#include <functional>

#define FMT_HEADER_ONLY
#include <cppformat/format.h>

#ifdef __cpp_constexpr
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
    auto log(int severity, const attributes_t& attributes, string_view format) const -> void;

    /// Log a message with the given severity level and attributes and further formatting using the
    /// given pattern and arguments.
    ///
    /// \overload
    /// \tparam T and Args... must meet the requirements of `StreamFormatted`.
    template<typename T, typename... Args>
    auto log(int severity, const attributes_t& attributes, string_view format, const T& arg, const Args&... args) const -> void;

#ifdef __cpp_constexpr
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
    auto log(int severity, const attributes_t& attributes, const detail::formatter<N>& formatter, const T& arg, const Args&... args) const -> void;
#endif

private:
    constexpr auto inner() const noexcept -> const wrapped_type& {
        return wrapped.get();
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
    const auto fn = [&](writer_t& wr) {
        wr.write(format.data(), arg, args...);
    };

    range_t range;
    inner().log(severity, format, range, std::cref(fn));
}

template<typename Logger>
inline
auto
logger_facade<Logger>::log(int severity, const attributes_t& attributes, string_view format) const -> void {
    range_t range{attributes};
    inner().log(severity, format, range);
}

template<typename Logger>
template<typename T, typename... Args>
inline
auto
logger_facade<Logger>::log(int severity, const attributes_t& attributes, string_view format, const T& arg, const Args&... args) const -> void {
    const auto fn = [&](writer_t& wr) {
        wr.write(format.data(), arg, args...);
    };

    range_t range{attributes};
    inner().log(severity, format, range, std::cref(fn));
}

#ifdef __cpp_constexpr

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
