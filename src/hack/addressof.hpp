#pragma once

#include <cstddef>

#include <boost/config.hpp>
#include <boost/version.hpp>
#include <boost/utility/addressof.hpp>

//! Workaround (honestly, a hack) for cases when Blackhole is being compiled with clang on systems
//! with boost 1.55 on board.
//!
//! Stolen from https://svn.boost.org/trac/boost/ticket/5487.
//!
//! \note must be included before <boost/variant/get.hpp> or <boost/utility/addressof.hpp>.

#if defined(__clang__) && (BOOST_VERSION / 100000 == 1 && BOOST_VERSION / 100 % 1000 == 55)

#ifndef BOOST_NO_CXX11_NULLPTR

namespace boost {
namespace detail {

#if defined(__clang__) && !defined(_LIBCPP_VERSION) && !defined(BOOST_NO_CXX11_DECLTYPE)
    typedef decltype(nullptr) addr_nullptr_t;
#else
    typedef std::nullptr_t addr_nullptr_t;
#endif

template<>
struct addressof_impl<addr_nullptr_t> {
    typedef addr_nullptr_t T;

    constexpr static T* f(T& v, int) {
        return &v;
    }
};

template<>
struct addressof_impl<addr_nullptr_t const> {
    typedef addr_nullptr_t const T;

    constexpr static T* f(T& v, int) {
        return &v;
    }
};

template<>
struct addressof_impl<addr_nullptr_t volatile> {
    typedef addr_nullptr_t volatile T;

    constexpr static T* f(T& v, int) {
        return &v;
    }
};

template<>
struct addressof_impl<addr_nullptr_t const volatile> {
    typedef addr_nullptr_t const volatile T;

    constexpr static T* f( T& v, int) {
        return &v;
    }
};

}  // namespace detail
}  // namespace boost

#endif // BOOST_NO_CXX11_NULLPTR

#endif
