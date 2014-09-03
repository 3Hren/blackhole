#pragma once

#include <iostream>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <boost/optional.hpp>

#include "blackhole/attribute/set.hpp"
#include "blackhole/config.hpp"
#include "blackhole/detail/iterator/join.hpp"

namespace blackhole {

namespace attribute {

class set_view_t {
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
    empty() const BLACKHOLE_NOEXCEPT {
        return empty<T>();
    }

    template<class T, class... Args>
    typename std::enable_if<
        sizeof...(Args) != 0 && sizeof...(Args) <= 3,
        bool
    >::type
    empty() const BLACKHOLE_NOEXCEPT {
        return empty<T>() && empty<Args...>();
    }

    void insert(pair_t pair);

    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last);

    const_iterator begin() const BLACKHOLE_NOEXCEPT;
    const_iterator end() const BLACKHOLE_NOEXCEPT;

    const set_t& external() const BLACKHOLE_NOEXCEPT {
        return external_.v;
    }

    boost::optional<const attribute_t&>
    find(const std::string& name) const BLACKHOLE_NOEXCEPT;

    const attribute_t& at(const std::string& name) const;
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
    return const_iterator({ &internal_.v, &external_.v, &attached_.v });
}

BLACKHOLE_API
set_view_t::const_iterator
set_view_t::end() const BLACKHOLE_NOEXCEPT {
    return const_iterator({ &internal_.v, &external_.v, &attached_.v }, aux::iterator::invalidate_tag);
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
