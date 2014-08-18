#pragma once

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

template<class Container, bool Const>
class iterator_t {
public:
    typedef Container container_type;

    typedef typename container_type::underlying_container underlying_container;

    typedef typename underlying_container::difference_type difference_type;
    typedef typename underlying_container::value_type value_type;

    typedef typename std::conditional<
        Const,
        typename underlying_container::const_reference,
        typename underlying_container::reference
    >::type reference;

    typedef typename std::conditional<
        Const,
        typename underlying_container::const_pointer,
        typename underlying_container::pointer
    >::type pointer;

    typedef typename std::conditional<
        Const,
        typename underlying_container::const_iterator,
        typename underlying_container::iterator
    >::type underlying_iterator;

    typedef typename std::conditional<
        Const,
        const underlying_container&,
        underlying_container&
    >::type container_reference_type;

    typedef std::forward_iterator_tag iterator_category;

private:
    friend class iterator_t<Container, !Const>;

    struct iterator_pair_t {
        underlying_iterator begin;
        underlying_iterator end;

        iterator_pair_t(container_reference_type container) :
            begin(container.begin()),
            end(container.end())
        {}

        iterator_pair_t(const iterator_pair_t& other) = default;
    };

    uint stage;
    std::vector<iterator_pair_t> iterators;
    underlying_iterator current;

public:
    iterator_t(container_reference_type c1,
               container_reference_type c2,
               container_reference_type c3,
               container_reference_type c4) :
        stage(0),
        iterators({ c1, c2, c3, c4 }),
        current(iterators.at(0).begin)
    {}

    iterator_t& operator++() BLACKHOLE_NOEXCEPT {
        BOOST_ASSERT(stage <= iterators.size());
        current++;
        if (current == iterators.at(stage).end) {
            if (stage != iterators.size() - 1) {
                current = iterators.at(stage + 1).begin;
            }

            stage++;
        }

        return *this;
    }

    pointer operator->() const BLACKHOLE_NOEXCEPT {
        return current.operator->();
    }

    reference operator*() const BLACKHOLE_NOEXCEPT {
        return *current;
    }

    bool valid() const BLACKHOLE_NOEXCEPT {
        BOOST_ASSERT(iterators.size());
        return current != iterators.back().end;
    }
};

// Provide get/set access.
// Can be iterated only forwardly.
class set_view_t {
public:
    typedef set_t underlying_container;

    typedef underlying_container::reference reference;
    typedef underlying_container::pointer   pointer;

    typedef iterator_t<set_view_t, false>   iterator;
    typedef iterator_t<set_view_t, true>    const_iterator;

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

    iterator
    iters() BLACKHOLE_NOEXCEPT {
        return iterator(other, local, scoped, global);
    }

    const_iterator
    iters() const BLACKHOLE_NOEXCEPT {
        return const_iterator(other, local, scoped, global);
    }

    boost::optional<attribute_t&>
    find(const std::string& name) BLACKHOLE_NOEXCEPT {
        auto it = other.find(name);
        if (it != other.end()) {
            return it->second;
        }

        it = local.find(name);
        if (it != local.end()) {
            return it->second;
        }

        it = scoped.find(name);
        if (it != scoped.end()) {
            return it->second;
        }

        it = global.find(name);
        if (it != global.end()) {
            return it->second;
        }

        return boost::optional<attribute_t&>();
    }

    boost::optional<const attribute_t&>
    find(const std::string& name) const BLACKHOLE_NOEXCEPT {
        return boost::optional<const attribute_t&>(const_cast<set_view_t*>(this)->find(name));
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
