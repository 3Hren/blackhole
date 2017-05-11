#pragma once

#include <functional>

#include "blackhole/attributes.hpp"
#include "blackhole/extensions/writer.hpp"
#include "blackhole/stdext/source_location.hpp"
#include "blackhole/stdext/string_view.hpp"

namespace blackhole {
inline namespace v1 {

namespace ph = std::placeholders;

/// Internal details. Please move along, nothing to see here.
namespace detail {
namespace gcc {

/// Workaround for GCC versions up to 4.9, which fails to expand variadic pack inside lambdas.
template<class... Args>
inline auto write_all(fmt::MemoryWriter& wr, const char* pattern, const Args&... args) -> string_view {
    wr.write(pattern, args...);
    return {wr.data(), wr.size()};
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
    auto apply(Logger& log, int severity, const string_view& pattern,
        const stdext::source_location& loc,
        const Args&... args,
        const attribute_list& attributes) -> void
    {
        fmt::MemoryWriter wr;
        const auto fn = std::bind(&gcc::write_all<Args...>, std::ref(wr), pattern.data(), std::cref(args)...);

        if (loc.is_valid()) {
            attribute_list loc_attributes{
                {"line", loc.line()},
                {"file", string_view(loc.file_name())},
            };
            attribute_pack pack{attributes, loc_attributes};
            log.log(severity, {pattern, std::cref(fn)}, pack);
        } else {
            attribute_pack pack{attributes};
            log.log(severity, {pattern, std::cref(fn)}, pack);
        }
    }
};

}  // namespace detail
}  // namespace v1
}  // namespace blackhole
