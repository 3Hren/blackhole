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
#include "blackhole/utils/noexcept.hpp"

namespace blackhole {

namespace attribute {

class iterator_t {
public:
    typedef std::forward_iterator_tag iterator_category;

private:
    mutable const set_t* cc;
    mutable size_t stage;
    mutable set_t::const_iterator ci;
    std::vector<const set_t*> cs;

public:
    iterator_t(const set_t* c1, const set_t* c2, const set_t* c3) :
        cc(c1),
        stage(0),
        ci(c1->begin()),
        cs({ c1, c2, c3 })
    {}

    iterator_t& operator++() BLACKHOLE_NOEXCEPT {
        ci++;
        if (ci == cc->end()) {
            if (stage < cs.size() - 1) {
                cc = cs[stage + 1];
                ci = cc->begin();
            }

            stage++;
        }

        return *this;
    }

    set_t::const_pointer operator->() const BLACKHOLE_NOEXCEPT {
        return ci.operator->();
    }

    set_t::const_reference operator*() const BLACKHOLE_NOEXCEPT {
        return *ci;
    }

    bool valid() const BLACKHOLE_NOEXCEPT {
        if (ci == cc->end()) {
            if (cc == cs.back()) {
                return false;
            }

            cc = cs[++stage];
            ci = cc->begin();
        }

        return true;
    }
};

class set_view_t {
    set_t global; // Likely empty.
    set_t local;  // About 1-2 + message + tid + timestamp (4-5).
    set_t other;  // The most filled (scoped + user attributes)

public:
    set_view_t() = default;
    set_view_t(set_t global, set_t scoped, set_t&& local) :
        global(std::move(global)),
        local(std::move(local)),
        other(std::move(scoped))
    {}

    bool empty() const BLACKHOLE_NOEXCEPT {
        return local.empty() && other.empty() && global.empty();
    }

    size_t count(const std::string& name) const BLACKHOLE_NOEXCEPT {
        return local.count(name) + other.count(name) + global.count(name);
    }

    size_t upper_size() const BLACKHOLE_NOEXCEPT {
        return local.size() + other.size() + global.size();
    }

    void insert(pair_t pair) {
        other.insert(std::move(pair));
    }

    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last) {
        other.insert(first, last);
    }

    iterator_t
    iters() const BLACKHOLE_NOEXCEPT {
        return iterator_t(&local, &other, &global);
    }

    boost::optional<const attribute_t&>
    find(const std::string& name) const BLACKHOLE_NOEXCEPT {
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

    const attribute_t& at(const std::string& name) const {
        auto value = find(name);
        if (!value) {
            throw std::out_of_range(name);
        }

        return *value;
    }
};

} // namespace attribute

} // namespace blackhole
