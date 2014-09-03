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
    typedef aux::iterator::join_t<set_t, true>  const_iterator;
    typedef set_t::size_type size_type;

private:
    set_t global;     // Likely empty.
    set_t internal;   // About level + message + pid + tid + timestamp (4-5).
    set_t external_;  // The most filled (scoped + user attributes)

public:
    set_view_t() = default;
    set_view_t(set_t global, set_t scoped, set_t&& internal);

    bool empty() const BLACKHOLE_NOEXCEPT;
    size_type upper_size() const BLACKHOLE_NOEXCEPT;
    size_type count(const std::string& name) const BLACKHOLE_NOEXCEPT;

    void insert(pair_t pair);

    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last);

    const_iterator begin() const BLACKHOLE_NOEXCEPT;
    const_iterator end() const BLACKHOLE_NOEXCEPT;

    const set_t& external() const BLACKHOLE_NOEXCEPT {
        return external_;
    }

    boost::optional<const attribute_t&>
    find(const std::string& name) const BLACKHOLE_NOEXCEPT;

    const attribute_t& at(const std::string& name) const;
};

BLACKHOLE_API
set_view_t::set_view_t(set_t global, set_t external, set_t&& internal) :
    global(std::move(global)),
    internal(std::move(internal)),
    external_(std::move(external))
{}

BLACKHOLE_API
bool
set_view_t::empty() const BLACKHOLE_NOEXCEPT {
    return internal.empty() && external_.empty() && global.empty();
}

BLACKHOLE_API
set_view_t::size_type
set_view_t::upper_size() const BLACKHOLE_NOEXCEPT {
    return internal.size() + external_.size() + global.size();
}

BLACKHOLE_API
set_view_t::size_type
set_view_t::count(const std::string& name) const BLACKHOLE_NOEXCEPT {
    return internal.count(name) + external_.count(name) + global.count(name);
}

BLACKHOLE_API
void
set_view_t::insert(pair_t pair) {
    external_.insert(std::move(pair));
}

template<typename InputIterator>
BLACKHOLE_API
void
set_view_t::insert(InputIterator first, InputIterator last) {
    external_.insert(first, last);
}

BLACKHOLE_API
set_view_t::const_iterator
set_view_t::begin() const BLACKHOLE_NOEXCEPT {
    return const_iterator({ &internal, &external_, &global });
}

BLACKHOLE_API
set_view_t::const_iterator
set_view_t::end() const BLACKHOLE_NOEXCEPT {
    return const_iterator({ &internal, &external_, &global }, aux::iterator::invalidate_tag);
}

BLACKHOLE_API
boost::optional<const attribute_t&>
set_view_t::find(const std::string& name) const BLACKHOLE_NOEXCEPT {
    auto it = internal.find(name);
    if (it != internal.end()) {
        return it->second;
    }

    it = external_.find(name);
    if (it != external_.end()) {
        return it->second;
    }

    it = global.find(name);
    if (it != global.end()) {
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
