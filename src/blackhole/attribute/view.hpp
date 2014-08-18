#pragma once

#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

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
    };

    uint stage;
    std::vector<iterator_pair_t> iterators;
    underlying_iterator current;

public:
    template<class... Containers>
    iterator_t(Containers&&... list) :
        stage(0),
        iterators(transform(std::forward<Containers>(list)...)),
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

private:
    template<class... Containers>
    static
    std::vector<iterator_pair_t>
    transform(Containers&&... args) {
        std::vector<iterator_pair_t> result;
        transform(result, std::forward<Containers>(args)...);
        return result;
    }

    static
    void
    transform(std::vector<iterator_pair_t>& r,
              container_reference_type arg)
    {
        r.push_back(iterator_pair_t(arg));
    }

    template<class... Containers>
    static
    void
    transform(std::vector<iterator_pair_t>& r,
              container_reference_type arg,
              Containers&&... args)
    {
        transform(r, arg);
        transform(r, std::forward<Containers>(args)...);
    }
};

// Provide get/set access.
// Can be iterated only forwardly.
class set_view_t {
public:
    typedef set_t underlying_container;

    typedef underlying_container::reference      reference;

    typedef underlying_container::iterator       iterator;
    typedef underlying_container::const_iterator const_iterator;

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
