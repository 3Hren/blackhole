#pragma once

#include <map>
#include <vector>

#include <boost/any.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/contains.hpp>

#include "blackhole/error.hpp"
#include "blackhole/repository/factory/traits.hpp"

namespace blackhole {

namespace aux {

template<typename T>
static bool is(const boost::any& any) {
    try {
        boost::any_cast<T>(any);
    } catch (const boost::bad_any_cast&) {
        return false;
    }

    return true;
}

template<typename T>
static void any_to(const boost::any& from, T& to) {
    to = boost::any_cast<T>(from);
}

typedef boost::mpl::vector<
    std::uint8_t,
    std::uint16_t,
    std::uint32_t,
    std::uint64_t,
    std::int8_t,
    std::int16_t,
    std::int32_t,
    std::int64_t
> supported_integers_t;

template<typename T>
struct is_supported_integer {
    static const bool value = boost::mpl::contains<supported_integers_t, T>::value;
};

template<typename T>
struct cast {
    const boost::any& source;
    T& value;
    bool& successful;

    cast(const boost::any& source, T& value, bool& successful) :
        source(source),
        value(value),
        successful(successful)
    {}

    template<typename Integer>
    void operator ()(Integer) {
        if (successful) {
            return;
        }

        const Integer* value_ptr = boost::any_cast<Integer>(&source);
        if (value_ptr) {
            value = *value_ptr;
            successful = true;
        }
    }
};

template<typename T, class = void> struct cast_traits;

template<typename T>
struct cast_traits<T, typename std::enable_if<!is_supported_integer<T>::value>::type> {
    static void to(const boost::any& source, T& value) {
        any_to(source, value);
    }
};

// When parsing external configs (like json) there are no information about
// the exact size of integer types.
// But for configs it is very unlikely to keep all integer variables under `int` without size
// specification, for example - port is always keeped as std::uint16_t.
// So, after parsing we have raw `int` type stored in boost::any.
// On the other side final config awaits concrete type. To connect them, I've decided to
// iterate over all supported integers in library (total 8) and try to convert boost::any to
// each of them in sequence until successful.
template<typename T>
struct cast_traits<T, typename std::enable_if<is_supported_integer<T>::value>::type> {
    static void to(const boost::any& source, T& value) {
        bool successful = false;
        boost::mpl::for_each<supported_integers_t>(cast<T>(source, value, successful));
        if (!successful) {
            throw blackhole::error_t("unsupported integer type");
        }
    }
};

template<typename Sink>
struct extractor {
    boost::any source;
    std::string name;

    extractor(const boost::any& source, const std::string& name = Sink::name()) :
        source(source),
        name(name)
    {}

    extractor<Sink> operator [](const std::string& name) const {
        std::map<std::string, boost::any> map;
        try {
            any_to(source, map);
        } catch (boost::bad_any_cast&) {
            throw error_t("can not extract '%s': '%s' is not map", name, this->name);
        }
        return extractor<Sink>(map[name], name);
    }

    template<typename T>
    void to(T& value) const {
        try{
            cast_traits<T>::to(source, value);
        } catch (boost::bad_any_cast&) {
            throw error_t("conversion error for member '%s'", name);
        }
    }

    template<typename T>
    T get() const {
        T value;
        to(value);
        return value;
    }
};

} // namespace aux

} // namespace blackhole
