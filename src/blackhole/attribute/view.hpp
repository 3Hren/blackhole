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
    //!@todo: Rename to '?', 'internal' 'external'.
    set_t global; // Likely empty.
    set_t local;  // About 1-2 + message + tid + timestamp (4-5).
    set_t other;  // The most filled (scoped + user attributes)

public:
    set_view_t() = default;
    set_view_t(set_t global, set_t scoped, set_t&& local);

    bool empty() const BLACKHOLE_NOEXCEPT;
    size_type upper_size() const BLACKHOLE_NOEXCEPT;
    size_type count(const std::string& name) const BLACKHOLE_NOEXCEPT;

    void insert(pair_t pair);

    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last);

    const_iterator begin() const BLACKHOLE_NOEXCEPT;
    const_iterator end() const BLACKHOLE_NOEXCEPT;

    boost::optional<const attribute_t&>
    find(const std::string& name) const BLACKHOLE_NOEXCEPT;

    const attribute_t& at(const std::string& name) const;
};

BLACKHOLE_API
set_view_t::set_view_t(set_t global, set_t scoped, set_t&& local) :
    global(std::move(global)),
    local(std::move(local)),
    other(std::move(scoped))
{}

BLACKHOLE_API
bool
set_view_t::empty() const BLACKHOLE_NOEXCEPT {
    return local.empty() && other.empty() && global.empty();
}

BLACKHOLE_API
set_view_t::size_type
set_view_t::upper_size() const BLACKHOLE_NOEXCEPT {
    return local.size() + other.size() + global.size();
}

BLACKHOLE_API
set_view_t::size_type
set_view_t::count(const std::string& name) const BLACKHOLE_NOEXCEPT {
    return local.count(name) + other.count(name) + global.count(name);
}

BLACKHOLE_API
void
set_view_t::insert(pair_t pair) {
    other.insert(std::move(pair));
}

template<typename InputIterator>
BLACKHOLE_API
void
set_view_t::insert(InputIterator first, InputIterator last) {
    other.insert(first, last);
}

BLACKHOLE_API
set_view_t::const_iterator
set_view_t::begin() const BLACKHOLE_NOEXCEPT {
    return const_iterator({ &local, &other, &global });
}

BLACKHOLE_API
set_view_t::const_iterator
set_view_t::end() const BLACKHOLE_NOEXCEPT {
    return const_iterator({ &local, &other, &global }, aux::iterator::invalidate_tag);
}

BLACKHOLE_API
boost::optional<const attribute_t&>
set_view_t::find(const std::string& name) const BLACKHOLE_NOEXCEPT {
    auto it = local.find(name);
    if (it != local.end()) {
        return it->second;
    }

    it = other.find(name);
    if (it != other.end()) {
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
