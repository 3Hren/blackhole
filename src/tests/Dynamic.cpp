#include "global.hpp"

#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>
#include <boost/variant.hpp>
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

class base_t {
    std::string type_;
    dynamic_t::object_t config;

public:
    base_t(std::string type) :
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

} // namespace blackhole

using namespace blackhole;

TEST(Dynamic, Class) {
    dynamic_t d;
    UNUSED(d)
}

TEST(Dynamic, CopyConstructible) {
    dynamic_t d(42);
    dynamic_t copy(d);
    EXPECT_EQ(42, d.to<int>());
    EXPECT_EQ(42, copy.to<int>());
}

TEST(Dynamic, CopyAssignable) {
    dynamic_t d(42);
    dynamic_t copy;
    copy = d;
    EXPECT_EQ(42, d.to<int>());
    EXPECT_EQ(42, copy.to<int>());
}

TEST(Dynamic, MoveConstructible) {
    dynamic_t d(42);
    dynamic_t moved(std::move(d));
    EXPECT_TRUE(d.invalid());
    EXPECT_EQ(42, moved.to<int>());
}

TEST(Dynamic, MoveAssignable) {
    dynamic_t d(42);
    dynamic_t moved;
    moved = std::move(d);
    EXPECT_TRUE(d.invalid());
    EXPECT_EQ(42, moved.to<int>());
}

TEST(Dynamic, ThrowsExceptionWhenPrecisionLossOccurs) {
    EXPECT_THROW(dynamic_t(256u).to<unsigned char>(), dynamic::precision_loss);
    EXPECT_THROW(dynamic_t(256).to<unsigned char>(), dynamic::precision_loss);
}

TEST(Dynamic, ThrowsExceptionOnInvalidCast) {
    EXPECT_THROW(dynamic_t(42.5).to<unsigned char>(), dynamic::bad_cast);
    EXPECT_THROW(dynamic_t("le shit").to<bool>(), dynamic::bad_cast);
}

TEST(Dynamic, ThrowsExceptionOnRequestingIndexFromNonArray) {
    EXPECT_THROW(dynamic_t(42.5)[1], dynamic::bad_cast);

    const dynamic_t d(42.5);
    EXPECT_THROW(d[1], dynamic::bad_cast);
}

TEST(Dynamic, ThrowsExceptionOnRequestingIndexFromNonObject) {
    EXPECT_THROW(dynamic_t(42.5)["key"], dynamic::bad_cast);

    const dynamic_t d(42.5);
    EXPECT_THROW(d["key"], dynamic::bad_cast);
}

TEST(Base, Class) {
    base_t base("string");
    UNUSED(base);
}

TEST(Base, GetType) {
    base_t base("string");
    EXPECT_EQ("string", base.type());
}

TEST(Base, LikeDictBool) {
    base_t base("stream");
    base["input"] = true;
    base["output"] = false;

    EXPECT_EQ(true, base["input"].to<bool>());
    EXPECT_EQ(false, base["output"].to<bool>());
}

TEST(Base, LikeDictUint8) {
    unsigned char actual = 42;

    base_t base("stream");
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<unsigned char>());
}

TEST(Base, LikeDictUint16) {
    unsigned short actual = 42;

    base_t base("stream");
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<unsigned short>());
}

TEST(Base, LikeDictUint32) {
    unsigned int actual = 42;

    base_t base("stream");
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<unsigned int>());
}

TEST(Base, LikeDictUint64) {
    unsigned long long actual = 42;

    base_t base("stream");
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<unsigned long long>());
}

TEST(Base, LikeDictInt8) {
    char actual = 42;

    base_t base("stream");
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<char>());
}

TEST(Base, LikeDictInt16) {
    short actual = 42;

    base_t base("stream");
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<short>());
}

TEST(Base, LikeDictInt32) {
    int actual = 42;

    base_t base("stream");
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<int>());
}

TEST(Base, LikeDictInt64) {
    long long actual = 42;

    base_t base("stream");
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<long long>());
}

TEST(Base, LikeDictDouble) {
    double actual = 42.100500;

    base_t base("stream");
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<double>());
}

TEST(Base, LikeDictString) {
    base_t base("stream");
    base["output"] = "stdout";

    EXPECT_EQ("stdout", base["output"].to<std::string>());
}

TEST(Base, LikeDictArray) {
    dynamic_t::array_t array;
    array.push_back(42);

    base_t base("stream");
    base["output"] = array;

    EXPECT_EQ(array, base["output"].to<dynamic_t::array_t>());
}

TEST(Base, LikeDictMap) {
    dynamic_t::object_t map;
    map["key"] = 42;

    base_t base("stream");
    base["output"] = map;

    EXPECT_EQ(map, base["output"].to<dynamic_t::object_t>());
}

TEST(Base, NestedMap) {
    base_t base("stream");
    base["output"]["verbose"] = true;

    EXPECT_EQ(true, base["output"]["verbose"].to<bool>());
}

TEST(Base, NestedArray) {
    base_t base("stream");
    base["output"][1] = true;

    EXPECT_EQ(true, base["output"][1].to<bool>());
}
