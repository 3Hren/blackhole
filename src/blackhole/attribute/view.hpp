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
#include "blackhole/detail/array.hpp"
#include "blackhole/detail/iterator/join.hpp"
#include "blackhole/detail/traits/tuple.hpp"

namespace blackhole {

namespace attribute {

template<class T>
struct extractor;

template<class... Args>
struct tuple_empty;

class set_view_t {
    template<class T> friend struct extractor;
    template<class... Args> friend struct tuple_empty;

public:
    typedef aux::iterator::join_t<set_t, true> const_iterator;

    struct attached_set_t { set_t v; };
    struct internal_set_t { set_t v; };
    struct external_set_t { set_t v; };

private:
    attached_set_t attached;  // Likely empty.
    internal_set_t internal;  // About level + message + pid + tid + timestamp (4-5).
    external_set_t external;  // The most filled (scoped + user attributes)

public:
    set_view_t() = default;

    set_view_t(set_t attached, set_t external, set_t&& internal) :
        attached({ std::move(attached) }),
        internal({ std::move(internal) }),
        external({ std::move(external) })
    {}

    bool
    empty() const BLACKHOLE_NOEXCEPT {
        return internal.v.empty() && external.v.empty() && attached.v.empty();
    }

    //! Intentionally allow to insert only into external attribute set.
    void insert(pair_t pair) {
        external.v.insert(std::move(pair));
    }

    //! Intentionally allow to insert only into external attribute set.
    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last) {
        external.v.insert(first, last);
    }

    const_iterator begin() const BLACKHOLE_NOEXCEPT {
        return const_iterator(
            aux::make_array(&internal.v, &external.v, &attached.v)
        );
    }

    const_iterator end() const BLACKHOLE_NOEXCEPT {
        return const_iterator::invalid(
            aux::make_array(&internal.v, &external.v, &attached.v)
        );
    }

    boost::optional<const attribute_t&>
    find(const std::string& name) const BLACKHOLE_NOEXCEPT {
        auto it = internal.v.find(name);
        if (it != internal.v.end()) {
            return it->second;
        }

        it = external.v.find(name);
        if (it != external.v.end()) {
            return it->second;
        }

        it = attached.v.find(name);
        if (it != attached.v.end()) {
            return it->second;
        }

        return boost::optional<const attribute_t&>();
    }

    const attribute_t& at(const std::string& name) const {
        auto value = find(name);
        if (!value) {
            throw std::out_of_range(name);
        }

        return *value;
    }
};

template<>
struct tuple_empty<> {
    static inline bool empty(const set_view_t&) {
        return true;
    }
};

template<>
struct tuple_empty<set_view_t::attached_set_t> {
    static inline bool empty(const set_view_t& view) {
        return view.attached.v.empty();
    }
};

template<>
struct tuple_empty<set_view_t::internal_set_t> {
    static inline bool empty(const set_view_t& view) {
        return view.internal.v.empty();
    }
};

template<>
struct tuple_empty<set_view_t::external_set_t> {
    static inline bool empty(const set_view_t& view) {
        return view.external.v.empty();
    }
};

template<class T, class... Args>
struct tuple_empty<T, Args...> {
    static inline bool empty(const set_view_t& view) {
        return tuple_empty<T>::empty(view) &&
                tuple_empty<Args...>::empty(view);
    }
};

template<>
struct extractor<set_view_t::attached_set_t> {
    static
    inline
    const set_t*
    extract(const set_view_t& view) {
        return &view.attached.v;
    }
};

template<>
struct extractor<set_view_t::internal_set_t> {
    static
    inline
    const set_t*
    extract(const set_view_t& view) {
        return &view.internal.v;
    }
};

template<>
struct extractor<set_view_t::external_set_t> {
    static
    inline
    const set_t*
    extract(const set_view_t& view) {
        return &view.external.v;
    }
};

template<class... T>
class partial_view_t {
public:
    typedef set_view_t::const_iterator const_iterator;

private:
    typedef std::array<const set_t*, sizeof...(T)> array_type;

    const set_view_t& view;

public:
    partial_view_t(const set_view_t& view) :
        view(view)
    {}

    bool
    empty() const BLACKHOLE_NOEXCEPT {
        return tuple_empty<T...>::empty(view);
    }

    const_iterator
    begin() const BLACKHOLE_NOEXCEPT {
        return const_iterator(
            array_type {{ extractor<T>::extract(view)... }}
        );
    }

    const_iterator
    end() const BLACKHOLE_NOEXCEPT {
        return const_iterator::invalid(
            array_type {{ extractor<T>::extract(view)... }}
        );
    }
};

} // namespace attribute

} // namespace blackhole
