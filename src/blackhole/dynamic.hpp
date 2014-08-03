#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <boost/variant.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include "blackhole/config.hpp"
#include "blackhole/detail/traits/integer.hpp"
#include "blackhole/utils/format.hpp"
#include "blackhole/utils/noexcept.hpp"
#include "blackhole/utils/nullptr.hpp"

namespace blackhole {

namespace conversion {

template<typename T, class Enable = void>
struct integer_t;

template<typename T>
struct integer_t<T, typename std::enable_if<type_traits::is_unsigned_integer<T>::value>::type> {
    typedef uint64_t type;
};

template<typename T>
struct integer_t<T, typename std::enable_if<type_traits::is_signed_integer<T>::value>::type> {
    typedef int64_t type;
};

} // namespace conversion

class dynamic_t {
public:
    struct null_t {
        bool operator==(const null_t&) const {
            return true;
        }
    };

    typedef bool                             bool_t;
    typedef int64_t                          int_t;
    typedef uint64_t                         uint_t;
    typedef double                           double_t;
    typedef std::string                      string_t;
    typedef std::vector<dynamic_t>           array_t;
    typedef std::map<std::string, dynamic_t> object_t;

    typedef boost::variant<
        null_t,
        bool_t,
        uint_t,
        int_t,
        double_t,
        string_t,
        array_t,
        object_t
    > value_type;

private:
    value_type value;

    template<typename T>
    struct is_convertible : public
        std::integral_constant<
            bool,
            std::is_convertible<dynamic_t::null_t,   T>::value ||
            std::is_convertible<dynamic_t::bool_t,   T>::value ||
            std::is_convertible<dynamic_t::uint_t,   T>::value ||
            std::is_convertible<dynamic_t::int_t,    T>::value ||
            std::is_convertible<dynamic_t::double_t, T>::value ||
            std::is_convertible<dynamic_t::string_t, T>::value ||
            std::is_convertible<dynamic_t::array_t,  T>::value ||
            std::is_convertible<dynamic_t::object_t, T>::value
        >
    {};

public:
    dynamic_t();
    dynamic_t(const dynamic_t& other);
    dynamic_t(dynamic_t&& other) BLACKHOLE_NOEXCEPT;

    dynamic_t(bool value);

    template<typename T>
    dynamic_t(T&& from,
              typename std::enable_if<
                  type_traits::is_integer<typename std::decay<T>::type>::value
              >::type* = 0);

    dynamic_t(double value);

    dynamic_t(const char* value);
    dynamic_t(std::string value);
    dynamic_t(array_t value);
    dynamic_t(object_t value);

    dynamic_t& operator=(const dynamic_t& other);
    dynamic_t& operator=(dynamic_t&& other) BLACKHOLE_NOEXCEPT;

    bool operator==(const dynamic_t& other) const;

    bool invalid() const;

    bool contains(const std::string& key) const;

    dynamic_t& operator[](array_t::size_type key);
    const dynamic_t& operator[](array_t::size_type key) const;
    dynamic_t& operator[](const std::string& key);
    const dynamic_t& operator[](const std::string& key) const;

    template<typename T>
    typename std::enable_if<
        is_convertible<T>::value,
        bool
    >::type
    is() const;

    template<typename T>
    typename std::enable_if<
        is_convertible<T>::value && !type_traits::is_integer<T>::value,
        T
    >::type
    to() const;

    template<typename T>
    typename std::enable_if<
        is_convertible<T>::value && type_traits::is_integer<T>::value,
        T
    >::type
    to() const;
};

namespace dynamic {

namespace visitor {

class name_t : public boost::static_visitor<std::string> {
public:
    std::string operator()(const dynamic_t::null_t&) const {
        return "null";
    }

    std::string operator()(const dynamic_t::bool_t&) const {
        return "bool";
    }

    std::string operator()(const dynamic_t::uint_t&) const {
        return "uint";
    }

    std::string operator()(const dynamic_t::int_t&) const {
        return "int";
    }

    std::string operator()(const dynamic_t::double_t&) const {
        return "double";
    }

    std::string operator()(const dynamic_t::string_t&) const {
        return "string";
    }

    std::string operator()(const dynamic_t::array_t&) const {
        return "array";
    }

    std::string operator()(const dynamic_t::object_t&) const {
        return "object";
    }
};

} // namespace visitor

class precision_loss : public std::out_of_range {
public:
    template<typename T>
    precision_loss(T actual,
                   const std::string& reason,
                   typename std::enable_if<
                       type_traits::is_integer<typename std::decay<T>::type>::value
                   >::type* = 0) :
        std::out_of_range(
            blackhole::utils::format(
                "unable to convert integer (%d) without precision loss: %s",
                actual,
                reason
            )
        )
    {}
};

class negative_overflow : public precision_loss {
public:
    template<typename T>
    negative_overflow(T actual) :
        precision_loss(actual, "negative overflow")
    {}
};

class positive_overflow : public precision_loss {
public:
    template<typename T>
    positive_overflow(T actual) :
        precision_loss(actual, "positive overflow")
    {}
};

class bad_cast : public std::logic_error {
public:
    bad_cast(const dynamic_t::value_type& value) :
        std::logic_error(
            blackhole::utils::format(
                "unable to convert dynamic type (underlying type is '%s')",
                boost::apply_visitor(dynamic::visitor::name_t(), value)
            )
        )
    {}
};

template<typename T, typename Actual>
static inline
typename std::enable_if<type_traits::is_integer<T>::value, T>::type
safe_cast(Actual actual) {
    try {
        return boost::numeric_cast<T>(actual);
    } catch (const boost::numeric::negative_overflow&) {
        throw dynamic::negative_overflow(actual);
    } catch (const boost::numeric::positive_overflow&) {
        throw dynamic::positive_overflow(actual);
    }
}

} // namespace dynamic

BLACKHOLE_API
dynamic_t::dynamic_t() :
    value(null_t())
{}

BLACKHOLE_API
dynamic_t::dynamic_t(const dynamic_t& other) :
    value(other.value)
{}

BLACKHOLE_API
dynamic_t::dynamic_t(dynamic_t&& other) BLACKHOLE_NOEXCEPT :
    value(std::move(other.value))
{
    other.value = null_t();
}

BLACKHOLE_API
dynamic_t::dynamic_t(bool value) :
    value(value)
{}

BLACKHOLE_API
dynamic_t::dynamic_t(double value) :
    value(value)
{}

BLACKHOLE_API
dynamic_t::dynamic_t(const char *value) :
    value(std::string(value))
{}

BLACKHOLE_API
dynamic_t::dynamic_t(std::string value) :
    value(std::move(value))
{}

BLACKHOLE_API
dynamic_t::dynamic_t(dynamic_t::array_t value) :
    value(std::move(value))
{}

BLACKHOLE_API
dynamic_t::dynamic_t(dynamic_t::object_t value) :
    value(std::move(value))
{}

template<typename T>
BLACKHOLE_API
dynamic_t::dynamic_t(T&& from,
                     typename std::enable_if<
                        type_traits::is_integer<typename std::decay<T>::type
                     >::value>::type*) :
    value(
        static_cast<
            typename conversion::integer_t<typename std::decay<T>::type>::type
        >(std::forward<T>(from))
    )
{}

BLACKHOLE_API
dynamic_t& dynamic_t::operator=(const dynamic_t& other) {
    this->value = other.value;
    return *this;
}

BLACKHOLE_API
dynamic_t& dynamic_t::operator=(dynamic_t&& other) BLACKHOLE_NOEXCEPT {
    this->value = std::move(other.value);
    other.value = null_t();
    return *this;
}

BLACKHOLE_API
bool dynamic_t::operator==(const dynamic_t& other) const {
    return value == other.value;
}

BLACKHOLE_API
bool dynamic_t::invalid() const {
    return is<null_t>();
}

BLACKHOLE_API
bool dynamic_t::contains(const std::string& key) const {
    auto object = to<object_t>();
    return object.find(key) != object.end();
}

BLACKHOLE_API
dynamic_t& dynamic_t::operator[](array_t::size_type key) {
    if (auto container = boost::get<array_t>(&value)) {
       if (key >= container->size()) {
           container->resize(key + 1);
       }
       return (*container)[key];
    }

    if (boost::get<null_t>(&value)) {
       value = array_t();
       return (*this)[key];
    }

    throw dynamic::bad_cast(value);
}

BLACKHOLE_API
const dynamic_t& dynamic_t::operator[](array_t::size_type key) const {
    if (auto container = boost::get<array_t>(&value)) {
       return container->at(key);
    }

    throw dynamic::bad_cast(value);
}

BLACKHOLE_API
dynamic_t& dynamic_t::operator[](const std::string& key) {
    if (auto map = boost::get<object_t>(&value)) {
        return (*map)[key];
    }

    if (boost::get<null_t>(&value)) {
        value = object_t();
        return (*this)[key];
    }

    throw dynamic::bad_cast(value);
}

BLACKHOLE_API
const dynamic_t& dynamic_t::operator[](const std::string &key) const {
    if (auto map = boost::get<object_t>(&value)) {
        return map->at(key);
    }

    throw dynamic::bad_cast(value);
}

template<typename T>
BLACKHOLE_API
typename std::enable_if<
    dynamic_t::is_convertible<T>::value,
    bool
>::type
dynamic_t::is() const {
    return boost::get<T>(&value) != nullptr;
}

template<typename T>
BLACKHOLE_API
typename std::enable_if<
    dynamic_t::is_convertible<T>::value && !type_traits::is_integer<T>::value,
    T
>::type
dynamic_t::to() const {
    if (auto result = boost::get<T>(&value)) {
        return *result;
    }

    throw dynamic::bad_cast(value);
}

template<typename T>
BLACKHOLE_API
typename std::enable_if<
    dynamic_t::is_convertible<T>::value && type_traits::is_integer<T>::value,
    T
>::type
dynamic_t::to() const {
    if (auto actual = boost::get<int_t>(&value)) {
        return dynamic::safe_cast<T>(*actual);
    }

    if (auto actual = boost::get<uint_t>(&value)) {
        return dynamic::safe_cast<T>(*actual);
    }

    throw dynamic::bad_cast(value);
}

} // namespace blackhole
