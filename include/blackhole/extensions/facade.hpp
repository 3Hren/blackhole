#pragma once

#include <functional>

#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304
#include "blackhole/sandbox.hpp"
#endif

#include "blackhole/extensions/facade.inl.hpp"

namespace blackhole {

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
    /// Creates a logger facade by wrapping the given logger.
    constexpr explicit logger_facade(const wrapped_type& wrapped) noexcept:
        wrapped(wrapped)
    {}

    /// Log a message with the given severity level.
    auto log(int severity, const string_view& format) const -> void;

    /// Log a message with the given severity level and attributes.
    ///
    /// \overload
    auto log(int severity, const string_view& format, const attribute_list& attributes) const -> void;

    /// Log a message with the given severity level and further formatting using the given pattern
    /// and arguments with optional attributes list as a last parameter.
    ///
    /// \overload
    /// \tparam T and Args... must meet the requirements of `StreamFormatted`.
    /// \note the last parameter of variadic `Args...` pack can be an `attribute_list`.
    template<typename T, typename... Args>
    auto log(int severity, const string_view& format, const T& arg, const Args&... args) const -> void;

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
logger_facade<Logger>::log(int severity, const string_view& format) const -> void {
    inner().log(severity, format);
}

template<typename Logger>
template<typename T, typename... Args>
inline
auto
logger_facade<Logger>::log(int severity, const string_view& format, const T& arg, const Args&... args) const -> void {
    select(severity, format, arg, args...);
}

template<typename Logger>
inline
auto
logger_facade<Logger>::log(int severity, const string_view& format, const attribute_list& attributes) const -> void {
    attribute_pack range{attributes};
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

    attribute_pack range;
    inner().log(severity, "", range, std::cref(fn));
}

#endif

template<typename Logger>
template<typename... Args>
inline
auto
logger_facade<Logger>::select(int severity, const string_view& format, const Args&... args) const ->
    typename std::enable_if<!detail::with_attributes<Args...>::value>::type
{
    const auto fn = std::bind(&detail::gcc::write_all<Args...>, ph::_1, format.data(), std::cref(args)...);

    attribute_pack range;
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
