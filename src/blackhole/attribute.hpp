#pragma once

#include <cstdint>
#include <ctime>
#include <initializer_list>
#include <unordered_map>
#include <iterator>

#include <boost/mpl/contains.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/variant.hpp>

#include "blackhole/platform/initializer_list.hpp"
#include "utils/timeval.hpp"
#include "utils/types.hpp"
#include "utils/underlying.hpp"

namespace blackhole {

namespace log {

typedef boost::variant<
    std::int32_t,
    std::uint32_t,
    long,
    unsigned long,
    std::int64_t,
    std::uint64_t,
    std::double_t,
    std::string,
    timeval
> attribute_value_t;

namespace attribute {

enum class scope : std::uint8_t {
    local       = 1 << 0,   /* user-defined event attributes */
    event       = 1 << 1,   /* not user-defined event attributes, like timestamp or message */
    global      = 1 << 2,   /* logger object attributes */
    thread      = 1 << 3,   /* thread attributes */
    universe    = 1 << 4    /* singleton attributes for entire application */
};

typedef aux::underlying_type<scope>::type scope_underlying_type;

static const scope DEFAULT_SCOPE = scope::local;

//! Helper metafunction that checks if the type `T` is compatible with attribute
//! internal implementation, i.e. `attribute_value_t` variant can be constructed
//! using type `T`.
//! @note: This metafunction ignores implicit type conversion.
template<typename T>
struct is_supported :
    public boost::mpl::contains<
        log::attribute_value_t::types,
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

} // namespace log

inline
log::attributes_t
merge(const std::initializer_list<log::attributes_t>& args) {
    typedef log::attributes_t value_type;

    //!@compat: Sadly, but std::initializer_list in GCC.4.4 has no typedefs.
#ifdef BLACKHOLE_INITIALIZER_LIST_HAS_TYPEDEFS
    typedef std::initializer_list<value_type>::const_iterator const_iterator;
#else
    typedef const value_type* const_iterator;
#endif

    typedef std::reverse_iterator<const_iterator> iterator;

    log::attributes_t summary;
    for (auto it = iterator(args.end()); it != iterator(args.begin()); ++it) {
        summary.insert(it->begin(), it->end());
    }

    return summary;
}

namespace attribute {

// Simple typedef for attributes initializer list. Useful when specifying local attributes.
typedef std::initializer_list<std::pair<std::string, log::attribute_value_t>> list;

// Dynamic attribute factory function.
template<typename T>
inline log::attribute_pair_t make(const std::string& name, const T& value, log::attribute::scope scope = log::attribute::DEFAULT_SCOPE) {
    return std::make_pair(name, log::attribute_t(value, scope));
}

template<typename T>
inline log::attribute_pair_t make(const std::string& name, T&& value, log::attribute::scope scope = log::attribute::DEFAULT_SCOPE) {
    return std::make_pair(name, log::attribute_t(std::move(value), scope));
}

// Attribute packing/unpacking/extracting.
template<typename T, class = void>
struct traits {
    static inline T pack(const T& value) {
        return value;
    }

    static inline T extract(const log::attributes_t& attributes, const std::string& name) {
        return boost::get<T>(attributes.at(name).value);
    }
};

template<typename T>
struct traits<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    typedef typename aux::underlying_type<T>::type underlying_type;

    static inline underlying_type pack(const T& value) {
        return static_cast<underlying_type>(value);
    }

    static inline T extract(const log::attributes_t& attributes, const std::string& name) {
        return static_cast<T>(boost::get<underlying_type>(attributes.at(name).value));
    }
};

} // namespace attribute

} // namespace blackhole
