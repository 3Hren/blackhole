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

#include "blackhole/common.hpp"
#include "blackhole/attribute/set.hpp"
#include "blackhole/utils/noexcept.hpp"
#include "blackhole/utils/timeval.hpp"

namespace blackhole {

namespace attribute {

struct invalidate_tag_t {};
static invalidate_tag_t invalidate_tag;

template<class Container, bool Const>
class join_iterator_t {
    friend class join_iterator_t<Container, !Const>;

public:
    typedef Container container_type;
    typedef typename container_type::value_type value_type;
    typedef typename container_type::difference_type difference_type;

    typedef typename std::conditional<
        Const,
        const container_type*,
        container_type*
    >::type container_pointer;

    typedef typename std::conditional<
        Const,
        typename container_type::const_pointer,
        typename container_type::pointer
    >::type pointer;

    typedef typename std::conditional<
        Const,
        typename container_type::const_iterator,
        typename container_type::iterator
    >::type iterator;

    typedef typename std::conditional<
        Const,
        typename container_type::const_reference,
        typename container_type::reference
    >::type reference;

    typedef typename std::iterator_traits<
        iterator
    >::iterator_category iterator_category;

    static_assert(
        (std::is_base_of<std::forward_iterator_tag, iterator_category>::value),
        "only forward iterators are supported"
    );

private:
    std::vector<container_pointer> containers;
    container_pointer current;
    iterator it;
    size_t pos;

public:
    BLACKHOLE_CONSTEXPR join_iterator_t() :
        current(nullptr),
        pos(0)
    {}

    join_iterator_t(std::initializer_list<container_pointer> list) :
        current(nullptr),
        pos(0)
    {
        init(list);
    }

    join_iterator_t(std::initializer_list<container_pointer> list, const invalidate_tag_t&) :
        current(nullptr),
        pos(0)
    {
        init(list);
        if (!containers.empty()) {
            current = containers.back();
            it = current->end();
        }
    }

    join_iterator_t& operator++() BLACKHOLE_NOEXCEPT {
        ++it;
        maybe_advance_stage();
        return *this;
    }

    join_iterator_t operator++(int) BLACKHOLE_NOEXCEPT {
        join_iterator_t tmp(*this);
        it++;
        maybe_advance_stage();
        return tmp;
    }

    pointer operator->() const BLACKHOLE_NOEXCEPT {
        return it.operator->();
    }

    reference operator*() const BLACKHOLE_NOEXCEPT {
        return *it;
    }

    template<bool flag>
    bool operator==(const join_iterator_t<container_type, flag>& other) const {
        if (containers.empty() && other.containers.empty()) {
            // Both iterators are invalid.
            return true;
        }

        return containers == other.containers && current == other.current && it == other.it;
    }

    template<bool flag>
    bool operator!=(const join_iterator_t<container_type, flag>& other) const {
        return !operator==(other);
    }

private:
    void init(const std::initializer_list<container_pointer>& list) {
        for (auto it = list.begin(); it != list.end(); ++it) {
            if (!(*it)->empty()) {
                containers.push_back(*it);
            }
        }

        if (!containers.empty()) {
            current = containers.front();
            this->it = current->begin();
        }
    }

    void maybe_advance_stage() BLACKHOLE_NOEXCEPT {
        if (it == current->end()) {
            if (current != containers.back()) {
                current = containers[pos + 1];
                it = current->begin();
            }

            pos++;
        }
    }
};

class set_view_t {
public:
    typedef join_iterator_t<set_t, true>  const_iterator;
    typedef join_iterator_t<set_t, false> iterator;

private:
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

    const_iterator
    begin() const BLACKHOLE_NOEXCEPT {
        return const_iterator({ &local, &other, &global });
    }

    const_iterator
    end() const BLACKHOLE_NOEXCEPT {
        return const_iterator({ &local, &other, &global }, invalidate_tag);
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
