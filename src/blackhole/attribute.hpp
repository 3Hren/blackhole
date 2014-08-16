#pragma once

#include <cstdint>
#include <ctime>
#include <initializer_list>
#include <unordered_map>
#include <iterator>
#include <stdexcept>

#include <boost/mpl/contains.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/variant.hpp>

#include "blackhole/attribute/scope.hpp"
#include "blackhole/attribute/value.hpp"

#include "blackhole/platform/initializer_list.hpp"
#include "utils/timeval.hpp"
#include "utils/types.hpp"
#include "utils/underlying.hpp"
#include "blackhole/utils/noexcept.hpp"

namespace blackhole {

namespace attribute {

//! Helper metafunction that checks if the type `T` is compatible with attribute
//! internal implementation, i.e. `attribute_value_t` variant can be constructed
//! using type `T`.
//! @note: This metafunction ignores implicit type conversion.
template<typename T>
struct is_supported :
    public boost::mpl::contains<
        attribute_value_t::types,
        typename std::decay<T>::type
    >
{};

// Helper metafunction that checks if `attribute_value_t` can be constructed using type `T`.
template<typename T>
struct is_constructible {
    typedef boost::mpl::vector<
        const char*,    // Implicit literal to string conversion.
        char,
        unsigned char,
        short,
        unsigned short
    > additional_types;

    typedef typename std::conditional<
        boost::mpl::contains<additional_types, typename std::decay<T>::type>::value || is_supported<T>::value,
        std::true_type,
        std::false_type
    >::type type;

    static const bool value = type::value;
};

} // namespace attribute

struct attribute_t {
    attribute_value_t value;
    attribute::scope scope;

    attribute_t() :
        value(std::uint8_t(0)),
        scope(attribute::DEFAULT_SCOPE)
    {}

    attribute_t(const attribute_value_t& value, attribute::scope type = attribute::DEFAULT_SCOPE) :
        value(value),
        scope(type)
    {}

    attribute_t(attribute_value_t&& value, attribute::scope type = attribute::DEFAULT_SCOPE) :
        value(std::move(value)),
        scope(type)
    {}

    // Force compiler to keep enum values according to its underlying type.
    // It is needed, cause for weakly-typed enums its underlying type and its values underlying
    // types may vary, which leads to exception while extracting from variant.
    template<typename T, class = typename std::enable_if<std::is_enum<T>::value>::type>
    attribute_t(T value, attribute::scope type = attribute::DEFAULT_SCOPE) :
        value(static_cast<typename aux::underlying_type<T>::type>(value)),
        scope(type)
    {}

    bool operator==(const attribute_t& other) const {
        return value == other.value && scope == other.scope;
    }
};

typedef std::string attribute_name_t;

typedef std::pair<
    attribute_name_t,
    attribute_t
> attribute_pair_t;

typedef std::unordered_map<
    attribute_pair_t::first_type,
    attribute_pair_t::second_type
> attributes_t;

template<class Container, bool Const>
class iterator_t {
    friend class iterator_t<Container, !Const>;

    typedef typename std::conditional<
        Const,
        typename attributes_t::const_iterator,
        typename attributes_t::iterator
    >::type underlying_iterator;

    typedef typename std::conditional<
        Const,
        const attributes_t&,
        attributes_t&
    >::type c;

    int stage;
    underlying_iterator it;
    underlying_iterator b1, e1;
    underlying_iterator b2, e2;
    underlying_iterator b3, e3;
    underlying_iterator b4, e4;

public:
    typedef Container container_type;
    typedef typename attributes_t::difference_type difference_type;
    typedef typename attributes_t::value_type value_type;

    typedef typename std::conditional<
        Const,
        typename attributes_t::const_reference,
        typename attributes_t::reference
    >::type reference;

    typedef typename std::conditional<
        Const,
        typename attributes_t::const_pointer,
        typename attributes_t::pointer
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
class attribute_set_view_t {
public:
    typedef attributes_t::iterator       iterator;
    typedef attributes_t::const_iterator const_iterator;

private:
    attributes_t scoped; // likely empty
    attributes_t global; // likely empty
    attributes_t local;  // 1-2
    attributes_t other;  // most filled.

public:
    attribute_set_view_t() = default;
    attribute_set_view_t(attributes_t global,
                         attributes_t scoped,
                         attributes_t&& local) :
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

    void insert(attribute_pair_t pair) {
        other.insert(std::move(pair));
    }

    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last) {
        other.insert(first, last);
    }

    iterator_t<attribute_set_view_t, true> begin() const BLACKHOLE_NOEXCEPT {
        return iterator_t<attribute_set_view_t, true>(other, local, scoped, global);
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
        return const_iterator(const_cast<attribute_set_view_t*>(this)->find(name));
    }

    const attribute_t& at(const std::string& name) const {
        auto it = find(name);
        if (it == end()) {
            throw std::out_of_range(name);
        }
        return it->second;
    }
};

namespace attribute {

// Simple typedef for attributes initializer list. Useful when specifying local attributes.
typedef std::initializer_list<std::pair<std::string, attribute_value_t>> list;

// Dynamic attribute factory function.
template<typename T>
inline attribute_pair_t make(const std::string& name, const T& value, attribute::scope scope = attribute::DEFAULT_SCOPE) {
    return std::make_pair(name, attribute_t(value, scope));
}

template<typename T>
inline attribute_pair_t make(const std::string& name, T&& value, attribute::scope scope = attribute::DEFAULT_SCOPE) {
    return std::make_pair(name, attribute_t(std::move(value), scope));
}

// Attribute packing/unpacking/extracting.
template<typename T, class = void>
struct traits {
    static inline T pack(const T& value) {
        return value;
    }

    static inline T extract(const attribute_set_view_t& attributes, const std::string& name) {
        return boost::get<T>(attributes.at(name).value);
    }
};

template<typename T>
struct traits<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    typedef typename aux::underlying_type<T>::type underlying_type;

    static inline underlying_type pack(const T& value) {
        return static_cast<underlying_type>(value);
    }

    static inline T extract(const attribute_set_view_t& attributes, const std::string& name) {
        return static_cast<T>(boost::get<underlying_type>(attributes.at(name).value));
    }
};

} // namespace attribute

} // namespace blackhole
