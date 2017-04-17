#pragma once

#include <functional>

#if (__GNUC__ >= 6 || defined(__clang__)) && defined(__cpp_constexpr) && __cpp_constexpr >= 201304
#include "blackhole/extensions/metaformat.hpp"
#endif

#include "blackhole/extensions/facade.inl.hpp"

namespace blackhole {
inline namespace v1 {

/// Logging facade wraps the underlying logger and provides convenient formatting methods.
///
/// \tparam Logger must meet the requirements of `Logger`.
template<typename Logger>
class logger_facade {
public:
    typedef Logger wrapped_type;

private:
    std::reference_wrapper<wrapped_type> wrapped;

public:
    /// Creates a logger facade by wrapping the given logger.
    constexpr explicit logger_facade(wrapped_type& wrapped) noexcept:
        wrapped(wrapped)
    {}

    /// Returns an lvalue reference to the the underlying logger.
    ///
    /// \note it should be constexpr, but C++11 automatically make this constant and conflicts with
    ///       the overload.
    auto inner() noexcept -> wrapped_type& {
        return wrapped.get();
    }

    /// Returns a const lvalue reference to the the underlying logger.
    ///
    /// \overload
    constexpr auto inner() const noexcept -> const wrapped_type& {
        return wrapped.get();
    }

    /// Log a message with the given severity level.
    auto log(int severity, const string_view& pattern) -> void;

    /// Log a message with the given severity level and attributes.
    ///
    /// \overload
    auto log(int severity, const string_view& pattern, const attribute_list& attributes) -> void;

    /// Log a message with the given severity level and further formatting using the given pattern
    /// and arguments with optional attributes list as a last parameter.
    ///
    /// \overload
    /// \tparam T and Args... must meet the requirements of `StreamFormatted`.
    /// \note the last parameter of variadic `Args...` pack can be an `attribute_list`.
    template<typename T, typename... Args>
    auto log(int severity, const string_view& pattern, const T& arg, const Args&... args) -> void;

#if (__GNUC__ >= 6 || defined(__clang__)) && defined(__cpp_constexpr) && __cpp_constexpr >= 201304
    /// Log a message with the given severity level and further formatting using the given pattern
    /// and arguments.
    ///
    /// This is more fast alternative comparing with default formatting, because the given pattern
    /// string is parsed into literals during compile time.
    ///
    /// \overload
    /// \tparam T and Args... must meet the requirements of `StreamFormatted`.
    template<std::size_t N, typename T, typename... Args>
    auto log(int severity, const detail::formatter<N>& pattern, const T& arg, const Args&... args) -> void;
#endif

private:
    /// Selects the proper method overload when using variadic pack interface.
    ///
    /// \overload for variadic pack without attribute list as the last argument.
    template<typename... Args>
    inline
    auto select(int severity, const string_view& pattern, const Args&... args) ->
        typename std::enable_if<!detail::with_attributes<Args...>::value>::type;

    /// Selects the proper method overload when using variadic pack interface.
    ///
    /// \overload for variadic pack with attribute list as the last argument.
    template<typename... Args>
    inline
    auto select(int severity, const string_view& pattern, const Args&... args) ->
        typename std::enable_if<detail::with_attributes<Args...>::value>::type;
};

template<typename Logger>
inline
auto
logger_facade<Logger>::log(int severity, const string_view& pattern) -> void {
    inner().log(severity, pattern);
}

template<typename Logger>
template<typename T, typename... Args>
inline
auto
logger_facade<Logger>::log(int severity, const string_view& pattern, const T& arg, const Args&... args) -> void {
    select(severity, pattern, arg, args...);
}

template<typename Logger>
inline
auto
logger_facade<Logger>::log(int severity, const string_view& pattern, const attribute_list& attributes) -> void {
    attribute_pack pack{attributes};
    inner().log(severity, pattern, pack);
}

#if (__GNUC__ >= 6 || defined(__clang__)) && defined(__cpp_constexpr) && __cpp_constexpr >= 201304

template<typename Logger>
template<std::size_t N, typename T, typename... Args>
inline
auto
logger_facade<Logger>::log(int severity, const detail::formatter<N>& pattern, const T& arg, const Args&... args) -> void {
    fmt::MemoryWriter wr;
    const auto fn = [&]() -> string_view {
        pattern.format(wr, arg, args...);
        return {wr.data(), wr.size()};
    };

    attribute_pack pack;
    // TODO: Pass non-empty pattern.
    inner().log(severity, {"", std::cref(fn)}, pack);
}

#endif

template<typename Logger>
template<typename... Args>
inline
auto
logger_facade<Logger>::select(int severity, const string_view& pattern, const Args&... args) ->
    typename std::enable_if<!detail::with_attributes<Args...>::value>::type
{
    fmt::MemoryWriter wr;
    const auto fn = std::bind(&detail::gcc::write_all<Args...>, std::ref(wr), pattern.data(), std::cref(args)...);

    attribute_pack pack;
    inner().log(severity, {pattern, std::cref(fn)}, pack);
}

template<typename Logger>
template<typename... Args>
inline
auto
logger_facade<Logger>::select(int severity, const string_view& pattern, const Args&... args) ->
    typename std::enable_if<detail::with_attributes<Args...>::value>::type
{
    detail::without_tail<detail::select_t, Args...>::type::apply(inner(), severity, pattern, args...);
}

} // namespace v1
} // namespace blackhole
