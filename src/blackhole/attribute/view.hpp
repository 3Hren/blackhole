#pragma once

#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "blackhole/attribute/set.hpp"
#include "blackhole/utils/noexcept.hpp"

namespace blackhole {

namespace attribute {

template<class Container, bool Const>
class iterator_t {
    friend class iterator_t<Container, !Const>;

    typedef typename std::conditional<
        Const,
        typename set_t::const_iterator,
        typename set_t::iterator
    >::type underlying_iterator;

    typedef typename std::conditional<
        Const,
        const set_t&,
        set_t&
    >::type c;

    int stage;
    underlying_iterator it;
    underlying_iterator b1, e1;
    underlying_iterator b2, e2;
    underlying_iterator b3, e3;
    underlying_iterator b4, e4;

public:
    typedef Container container_type;
    typedef typename attribute::set_t::difference_type difference_type;
    typedef typename attribute::set_t::value_type value_type;

    typedef typename std::conditional<
        Const,
        typename attribute::set_t::const_reference,
        typename attribute::set_t::reference
    >::type reference;

    typedef typename std::conditional<
        Const,
        typename set_t::const_pointer,
        typename set_t::pointer
    >::type pointer;

    typedef std::forward_iterator_tag iterator_category;

public:
    iterator_t(c c1, c c2, c c3, c c4) BLACKHOLE_NOEXCEPT :
        stage(0),
        it(c1.begin()),
        b1(c1.begin()), e1(c1.end()),
        b2(c2.begin()), e2(c2.end()),
        b3(c3.begin()), e3(c3.end()),
        b4(c4.begin()), e4(c4.end())
    {}

    iterator_t& operator++() BLACKHOLE_NOEXCEPT {
        it++;
        switch (stage) {
        case 0:
            if (it == e1) { it = b2; stage++; }
            break;
        case 1:
            if (it == e2) { it = b3; stage++; }
            break;
        case 2:
            if (it == e3) { it = b4; stage++; }
            break;
        case 3:
            if (it == e4) { stage++; }
            break;
        default:
            BOOST_ASSERT(false);
        }

        return *this;
    }

    pointer operator->() const BLACKHOLE_NOEXCEPT {
        return it.operator->();
    }

    reference operator*() const BLACKHOLE_NOEXCEPT {
        return *it;
    }

    bool valid() const {
        return it != e4;
    }
};

// Provide get/set access.
// Can be iterated only forwardly.
class set_view_t {
public:
    typedef set_t::iterator       iterator;
    typedef set_t::const_iterator const_iterator;

private:
    set_t scoped; // likely empty
    set_t global; // likely empty
    set_t local;  // 1-2
    set_t other;  // most filled.

public:
    set_view_t() = default;
    set_view_t(set_t global, set_t scoped, set_t&& local) :
        scoped(std::move(scoped)),
        global(std::move(global)),
        local(std::move(local))
    {
        //!@compat GCC > 4.4: other.reserve(8);
    }

    bool empty() const BLACKHOLE_NOEXCEPT {
        return other.empty() && local.empty() && scoped.empty() && global.empty();
    }

    size_t count(const std::string& name) const BLACKHOLE_NOEXCEPT {
        return other.count(name) + local.count(name) + scoped.count(name) + global.count(name);
    }

    //!@todo: Rename to `upper_size()` or something else.
    size_t size() const BLACKHOLE_NOEXCEPT {
        return other.size() + local.size() + scoped.size() + global.size();
    }

    void insert(pair_t pair) {
        other.insert(std::move(pair));
    }

    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last) {
        other.insert(first, last);
    }

    iterator_t<set_view_t, true> begin() const BLACKHOLE_NOEXCEPT {
        return iterator_t<set_view_t, true>(other, local, scoped, global);
    }

    iterator end() BLACKHOLE_NOEXCEPT {
        return other.end();
    }

    const_iterator end() const BLACKHOLE_NOEXCEPT {
        return other.end();
    }

    iterator find(const std::string& name) BLACKHOLE_NOEXCEPT {
        auto it = other.find(name);
        if (it != other.end()) {
            return it;
        }

        it = local.find(name);
        if (it != local.end()) {
            return it;
        }

        it = scoped.find(name);
        if (it != scoped.end()) {
            return it;
        }

        it = global.find(name);
        if (it != global.end()) {
            return it;
        }

        return end();
    }

    const_iterator find(const std::string& name) const BLACKHOLE_NOEXCEPT {
        return const_iterator(const_cast<set_view_t*>(this)->find(name));
    }

    const attribute_t& at(const std::string& name) const {
        auto it = find(name);
        if (it == end()) {
            throw std::out_of_range(name);
        }
        return it->second;
    }
};

} // namespace attribute

} // namespace blackhole
