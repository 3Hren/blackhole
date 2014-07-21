#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/any.hpp>
#include <boost/variant.hpp>

#include "blackhole/config.hpp"
#include "blackhole/detail/traits/integer.hpp"
#include "blackhole/error.hpp"
#include "blackhole/repository/factory/traits/cast.hpp"
#include "blackhole/utils/format.hpp"
#include "blackhole/utils/noexcept.hpp"
#include "blackhole/utils/nullptr.hpp"

namespace blackhole {

namespace repository {

namespace config {

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

    dynamic_t& operator[](array_t::size_type key);
    const dynamic_t& operator[](array_t::size_type key) const;
    dynamic_t& operator[](const std::string& key);
    const dynamic_t& operator[](const std::string& key) const;

    template<typename T>
    typename std::enable_if<!type_traits::is_integer<T>::value, T>::type
    to() const;

    template<typename T>
    typename std::enable_if<type_traits::is_integer<T>::value, T>::type
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

class precision_loss : public std::logic_error {
public:
    template<typename T>
    precision_loss(T actual,
                   typename std::enable_if<
                       type_traits::is_integer<typename std::decay<T>::type>::value
                   >::type* = 0) :
        std::logic_error(
            blackhole::utils::format(
                "unable to convert integer (%d) without precision loss",
                actual
            )
        )
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
    T converted = static_cast<T>(actual);
    if (actual != static_cast<Actual>(converted)) {
        throw dynamic::precision_loss(actual);
    }

    return converted;
}

} // namespace dynamic

dynamic_t::dynamic_t() :
    value(null_t())
{}

dynamic_t::dynamic_t(const dynamic_t& other) :
    value(other.value)
{}

dynamic_t::dynamic_t(dynamic_t&& other) BLACKHOLE_NOEXCEPT :
    value(std::move(other.value))
{
    other.value = null_t();
}

dynamic_t::dynamic_t(bool value) :
    value(value)
{}

dynamic_t::dynamic_t(double value) :
    value(value)
{}

dynamic_t::dynamic_t(const char *value) :
    value(std::string(value))
{}

dynamic_t::dynamic_t(std::string value) :
    value(std::move(value))
{}

dynamic_t::dynamic_t(dynamic_t::array_t value) :
    value(std::move(value))
{}

dynamic_t::dynamic_t(dynamic_t::object_t value) :
    value(std::move(value))
{}

template<typename T>
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

dynamic_t& dynamic_t::operator=(const dynamic_t& other) {
    this->value = other.value;
    return *this;
}

dynamic_t& dynamic_t::operator=(dynamic_t&& other) BLACKHOLE_NOEXCEPT {
    this->value = std::move(other.value);
    other.value = null_t();
    return *this;
}

bool dynamic_t::operator==(const dynamic_t& other) const {
    return value == other.value;
}

bool dynamic_t::invalid() const {
    return boost::get<null_t>(&value) != nullptr;
}

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

const dynamic_t& dynamic_t::operator[](array_t::size_type key) const {
    if (auto container = boost::get<array_t>(&value)) {
       return container->at(key);
    }

    throw dynamic::bad_cast(value);
}

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

const dynamic_t& dynamic_t::operator[](const std::string &key) const {
    if (auto map = boost::get<object_t>(&value)) {
        return map->at(key);
    }

    throw dynamic::bad_cast(value);
}

template<typename T>
typename std::enable_if<!type_traits::is_integer<T>::value, T>::type
dynamic_t::to() const {
    if (auto result = boost::get<T>(&value)) {
        return *result;
    }

    throw dynamic::bad_cast(value);
}

template<typename T>
typename std::enable_if<type_traits::is_integer<T>::value, T>::type
dynamic_t::to() const {
    if (auto actual = boost::get<int_t>(&value)) {
        return dynamic::safe_cast<T>(*actual);
    }

    if (auto actual = boost::get<uint_t>(&value)) {
        return dynamic::safe_cast<T>(*actual);
    }

    throw dynamic::bad_cast(value);
}

class base_tV2 {
    std::string type_;
    dynamic_t::object_t config;

public:
    base_tV2(std::string type) :
        type_(std::move(type))
    {}

    std::string type() const {
        return type_;
    }

    dynamic_t&
    operator[](const std::string& key) {
        return config[key];
    }

    const dynamic_t&
    operator[](const std::string& key) const {
        return config.at(key);
    }
};

typedef boost::any holder_type;
typedef std::vector<holder_type> array_type;
typedef std::map<std::string, holder_type> map_type;

//!@todo: Write documentation.
struct base_t {
    std::string type;
    holder_type config;

    base_t(std::string type) :
        type(std::move(type)),
        config(map_type())
    {}

    template<typename T>
    base_t& operator=(const T& value) {
        config = value;
        return *this;
    }

    struct writer_t {
        holder_type& holder;

        template<typename T>
        writer_t& operator=(const T& value) {
            holder = value;
            return *this;
        }

        writer_t& operator=(const char* value) {
            return operator=(std::string(value));
        }

        writer_t operator[](array_type::size_type id) {
            auto array = boost::any_cast<array_type>(&holder);
            if (!array) {
                holder = array_type();
                array = boost::any_cast<array_type>(&holder);
            }
            if (id >= array->size()) {
                array->resize(id + 1);
            }
            return writer_t { array->at(id) };
        }

        writer_t operator[](const std::string& name) {
            auto map = boost::any_cast<map_type>(&holder);
            if (!map) {
                holder = map_type();
                map = boost::any_cast<map_type>(&holder);
            }

            if (map->find(name) == map->end()) {
                map->insert(std::make_pair(name, holder_type()));
            }

            return writer_t { map->at(name) };
        }
    };

    struct reader_t {
        const holder_type& holder;

        reader_t operator[](array_type::size_type id) const {
            auto array = boost::any_cast<array_type>(&holder);
            if (!array) {
                throw blackhole::error_t("not array");
            }

            return reader_t { array->at(id) };
        }

        reader_t operator [](const std::string& name) const {
            auto map = boost::any_cast<map_type>(&holder);
            if (!map) {
                throw blackhole::error_t("not map");
            }

            return reader_t { map->at(name) };
        }

        template<typename T>
        void to(T& value) const {
            try {
                aux::any_to(holder, value);
            } catch (boost::bad_any_cast&) {
                throw error_t("conversion error");
            }
        }

        template<typename T>
        T to() const {
            T value;
            to(value);
            return value;
        }
    };

    writer_t operator[](array_type::size_type id) {
        auto& array = *boost::any_cast<array_type>(&config);
        return writer_t { array[id] };
    }

    writer_t operator[](const std::string& name) {
        auto& map = *boost::any_cast<map_type>(&config);
        return writer_t { map[name] };
    }

    reader_t operator[](array_type::size_type id) const {
        const auto& array = *boost::any_cast<array_type>(&config);
        return reader_t { array.at(id) };
    }

    reader_t operator[](const std::string& name) const {
        const auto& map = *boost::any_cast<map_type>(&config);
        return reader_t { map.at(name) };
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
