#pragma once

#include <functional>

#include "blackhole/attributes.hpp"
#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/extensions/writer.hpp"

namespace blackhole {

namespace ph = std::placeholders;

/// Internal details. Please move along, nothing to see here.
namespace detail {
namespace gcc {

/// Workaround for GCC versions up to 4.9, which fails to expand variadic pack inside lambdas.
template<class... Args>
inline void write_all(writer_t& wr, const char* pattern, const Args&... args) {
    wr.write(pattern, args...);
}

}  // namespace gcc

/// Helper metafunction that deduces the last type from the given variadic pack.
///
/// For example:
///     last_of<int>::type         -> int.
///     last_of<int, double>::type -> double.
template<typename... Tail>
struct last_of;

/// Forbid unpacking empty variadic pack.
///
/// \note the same behavior can be achieved using another type parameter prepending variadic pack,
///       but GCC fails to compile it with `sorry, unimplemented`.
template<>
struct last_of<>;

template<typename T>
struct last_of<T> {
    typedef T type;
};

template<typename T, typename U, typename... Tail>
struct last_of<T, U, Tail...> {
    typedef typename last_of<U, Tail...>::type type;
};

/// Helper metafunction that determines whether the last parameter from given variadic pack is an
/// attributes list.
///
/// For example:
///     with_attributes<int, double>::type                 -> std::false_type.
///     with_attributes<int, double, attribute_list>::type -> std::true_type.
template<typename... Args>
struct with_attributes : public std::is_same<typename last_of<Args...>::type, attribute_list> {};

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
    auto apply(Logger& log, int severity, const string_view& pattern, const Args&... args,
        const attribute_list& attributes) -> void
    {
        const auto fn = std::bind(&gcc::write_all<Args...>, ph::_1, pattern.data(), std::cref(args)...);

        attribute_pack pack{attributes};
        log.log(severity, pattern, pack, std::cref(fn));
    }
};

}  // namespace detail
}  // namespace blackhole
