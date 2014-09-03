#pragma once

#include <iterator>
#include <type_traits>
#include <vector>

#include "blackhole/detail/config/constexpr.hpp"
#include "blackhole/detail/config/noexcept.hpp"
#include "blackhole/detail/config/nullptr.hpp"

namespace blackhole {

namespace aux {

namespace iterator {

struct invalidate_tag_t {};
static invalidate_tag_t invalidate_tag;

template<class Container, bool Const>
class join_t {
    friend class join_t<Container, !Const>;

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
    template<class T>
    join_t(const T& array) :
        current(nullptr),
        pos(0)
    {
        init(array);
    }

    template<class T>
    join_t(const T& array, const invalidate_tag_t&) :
        current(nullptr),
        pos(0)
    {
        init(array);
        if (!containers.empty()) {
            current = containers.back();
            it = current->end();
        }
    }

    join_t& operator++() BLACKHOLE_NOEXCEPT {
        ++it;
        maybe_advance_stage();
        return *this;
    }

    join_t operator++(int) BLACKHOLE_NOEXCEPT {
        join_t tmp(*this);
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
    bool operator==(const join_t<container_type, flag>& other) const {
        if (containers.empty() && other.containers.empty()) {
            // Both iterators are invalid.
            return true;
        }

        return containers == other.containers && current == other.current && it == other.it;
    }

    template<bool flag>
    bool operator!=(const join_t<container_type, flag>& other) const {
        return !operator==(other);
    }

private:
    template<class T>
    void init(const T& list) {
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

} // namespace iterator

} // namespace aux

} // namespace blackhole
