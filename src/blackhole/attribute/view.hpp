#pragma once

#include <array>
#include <iostream>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <boost/mpl/set.hpp>
#include <boost/optional.hpp>

#include "blackhole/attribute/set.hpp"
#include "blackhole/config.hpp"
#include "blackhole/detail/iterator/join.hpp"
#include "blackhole/detail/traits/tuple.hpp"

namespace blackhole {

namespace attribute {

template<class... T>
struct pvex;

class set_view_t {
    template<class... T> friend struct pvex;

public:
    typedef aux::iterator::join_t<set_t, true> const_iterator;

    struct attached_set_t { set_t v; };
    struct internal_set_t { set_t v; };
    struct external_set_t { set_t v; };

private:
    attached_set_t attached_;  // Likely empty.
    internal_set_t internal_;  // About level + message + pid + tid + timestamp (4-5).
    external_set_t external_;  // The most filled (scoped + user attributes)

public:
    set_view_t() = default;
    set_view_t(set_t attached, set_t external, set_t&& internal);

    bool
    empty() const BLACKHOLE_NOEXCEPT {
        return empty<attached_set_t, internal_set_t, external_set_t>();
    }

    template<class T>
    bool
    empty() const BLACKHOLE_NOEXCEPT;

    template<class T, class... Args>
    typename std::enable_if<
        sizeof...(Args) != 0 && sizeof...(Args) <= 3,
        bool
    >::type
    empty() const BLACKHOLE_NOEXCEPT {
        return empty<T>() && empty<Args...>();
    }

    //! Intentionally allow to insert only to external attribute set.
    void insert(pair_t pair);

    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last);

    const_iterator begin() const BLACKHOLE_NOEXCEPT;
    const_iterator end() const BLACKHOLE_NOEXCEPT;

    boost::optional<const attribute_t&>
    find(const std::string& name) const BLACKHOLE_NOEXCEPT;

    const attribute_t& at(const std::string& name) const;
};

template<>
struct pvex<set_view_t::attached_set_t> {
    static
    std::tuple<set_view_t::attached_set_t>
    init(const set_view_t& view) {
        return std::make_tuple(view.attached_);
    }
};

template<>
struct pvex<set_view_t::internal_set_t> {
    static
    std::tuple<set_view_t::internal_set_t>
    init(const set_view_t& view) {
        return std::make_tuple(view.internal_);
    }
};

template<>
struct pvex<set_view_t::external_set_t> {
    static
    std::tuple<set_view_t::external_set_t>
    init(const set_view_t& view) {
        return std::make_tuple(view.external_);
    }
};

template<class T, class Arg, class... Args>
struct pvex<T, Arg, Args...> {
    static
    std::tuple<T, Arg, Args...>
    init(const set_view_t& view) {
        return std::tuple_cat(pvex<T>::init(view), pvex<Arg, Args...>::init(view));
    }
};

typedef set_view_t::const_iterator::container_pointer container_pointer;

/// A type that represents a parameter pack of zero or more integers.
template<unsigned... Indices>
struct index_tuple {
    /// Generate an index_tuple with an additional element.
    template<unsigned N>
    struct append {
        typedef index_tuple<Indices..., N> type;
    };
};

/// Unary metafunction that generates an index_tuple containing [0, Size)
template<unsigned Size>
struct make_index_tuple {
    typedef typename make_index_tuple<Size - 1>::type::template append<Size - 1>::type type;
};

// Terminal case of the recursive metafunction.
template<>
struct make_index_tuple<0u> {
    typedef index_tuple<> type;
};

template<typename... U>
struct array_t {
    typedef std::array<const set_t*, sizeof...(U) +  1> type;
};

template<typename T, typename... U, unsigned... I>
inline
typename array_t<U...>::type
toa(const std::tuple<T, U...>& t, index_tuple<I...>) {
    return typename array_t<U...>::type{{ &(std::get<I>(t).v)... }};
}

template<class T, unsigned size>
struct all_empty_t {
    static bool empty(const T& tuple) {
        return std::get<size>(tuple).v.empty() && all_empty_t<T, size - 1>::empty(tuple);
    }
};

template<class T>
struct all_empty_t<T, 0u> {
    static bool empty(const T& tuple) {
        return std::get<0>(tuple).v.empty();
    }
};

template<class... U>
class partial_view_t {
    std::tuple<U...> tuple;

public:
    partial_view_t(const set_view_t& view) :
        tuple(pvex<U...>::init(view))
    {}

    bool
    empty() const BLACKHOLE_NOEXCEPT {
        return all_empty_t<std::tuple<U...>, sizeof...(U) - 1>::empty(tuple);
    }

    set_view_t::const_iterator
    begin() {
        typedef typename make_index_tuple<sizeof...(U)>::type IndexTuple;
        auto a = toa(tuple, IndexTuple());
        return set_view_t::const_iterator(a);
    }

    set_view_t::const_iterator
    end() {
        typedef typename make_index_tuple<sizeof...(U)>::type IndexTuple;
        auto list = toa(tuple, IndexTuple());
        return set_view_t::const_iterator(list, aux::iterator::invalidate_tag);
    }
};

BLACKHOLE_API
set_view_t::set_view_t(set_t attached, set_t external, set_t&& internal) :
    attached_({ std::move(attached) }),
    internal_({ std::move(internal) }),
    external_({ std::move(external) })
{}

template<>
BLACKHOLE_API
bool
set_view_t::empty<set_view_t::attached_set_t>() const BLACKHOLE_NOEXCEPT {
    return attached_.v.empty();
}

template<>
BLACKHOLE_API
bool
set_view_t::empty<set_view_t::internal_set_t>() const BLACKHOLE_NOEXCEPT {
    return internal_.v.empty();
}

template<>
BLACKHOLE_API
bool
set_view_t::empty<set_view_t::external_set_t>() const BLACKHOLE_NOEXCEPT {
    return external_.v.empty();
}

BLACKHOLE_API
void
set_view_t::insert(pair_t pair) {
    external_.v.insert(std::move(pair));
}

template<typename InputIterator>
BLACKHOLE_API
void
set_view_t::insert(InputIterator first, InputIterator last) {
    external_.v.insert(first, last);
}

BLACKHOLE_API
set_view_t::const_iterator
set_view_t::begin() const BLACKHOLE_NOEXCEPT {
    std::array<set_t const*, 3> a{{ &internal_.v, &external_.v, &attached_.v }};
    return const_iterator(a);
}

BLACKHOLE_API
set_view_t::const_iterator
set_view_t::end() const BLACKHOLE_NOEXCEPT {
    std::array<set_t const*, 3> a{{ &internal_.v, &external_.v, &attached_.v }};
    return const_iterator(a, aux::iterator::invalidate_tag);
}

BLACKHOLE_API
boost::optional<const attribute_t&>
set_view_t::find(const std::string& name) const BLACKHOLE_NOEXCEPT {
    auto it = internal_.v.find(name);
    if (it != internal_.v.end()) {
        return it->second;
    }

    it = external_.v.find(name);
    if (it != external_.v.end()) {
        return it->second;
    }

    it = attached_.v.find(name);
    if (it != attached_.v.end()) {
        return it->second;
    }

    return boost::optional<const attribute_t&>();
}

BLACKHOLE_API
const attribute_t&
set_view_t::at(const std::string &name) const {
    auto value = find(name);
    if (!value) {
        throw std::out_of_range(name);
    }

    return *value;
}

} // namespace attribute

} // namespace blackhole
