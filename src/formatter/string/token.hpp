#pragma once

#include <string>

#include <boost/variant/variant.hpp>

#include "../../datetime.hpp"

namespace blackhole {
inline namespace v1 {
namespace formatter {
namespace string {

/// Helper named structs for making eye-candy code.
struct id;
struct hex;
struct num;
struct name;
struct user;
struct value;
struct required;
struct optional;

/// Represents string literal.
struct literal_t {
    std::string value;

    literal_t() = default;
    literal_t(std::string value) : value(std::move(value)) {}
};

namespace placeholder {

template<typename T>
struct generic;

template<>
struct generic<required> {
    std::string name;
    std::string spec;

    generic(std::string name);
    generic(std::string name, std::string spec);
};

template<>
struct generic<optional> : public generic<required> {
    std::string prefix;
    std::string suffix;

    boost::variant<
        std::int64_t,
        double,
        std::string
    > otherwise;

    generic(std::string name);
    generic(std::string name, std::string spec);
    generic(generic<required> token, std::string prefix, std::string suffix);
};

struct message_t {
    std::string spec;

    message_t();
    message_t(std::string spec);
};

template<typename T>
struct severity {
    std::string spec;

    severity();
    severity(std::string spec);
};

template<typename T>
struct timestamp;

template<>
struct timestamp<num> {
    std::string spec;

    timestamp();
    timestamp(std::string spec);
};

template<>
struct timestamp<user> {
    std::string pattern;
    std::string spec;
    bool gmtime;
    datetime::generator_t generator;

    timestamp();
    timestamp(std::string pattern, std::string spec, bool gmtime);
};

template<typename T>
struct process {
    std::string spec;

    process();
    process(std::string spec);
};

template<typename T>
struct thread {
    std::string spec;

    thread();
    thread(std::string spec);
};

template<>
struct thread<hex> {
    std::string spec;

    thread();
    thread(std::string spec);
};

template<typename T>
struct attribute {
    std::string spec;
    std::string format;

    attribute();
    attribute(std::string spec);
};

struct leftover_t {
    typedef boost::variant<
        literal_t,
        placeholder::attribute<name>,
        placeholder::attribute<value>
    > token_t;

    std::string prefix;
    std::string suffix;

    std::string spec;
    std::string separator;
    std::vector<token_t> tokens;

    leftover_t();
};

extern template struct severity<num>;
extern template struct severity<user>;
extern template struct process<id>;
extern template struct process<name>;
extern template struct thread<id>;
extern template struct thread<name>;
extern template struct attribute<name>;
extern template struct attribute<value>;

}  // namespace placeholder

namespace ph = placeholder;

typedef boost::variant<
    literal_t,
    ph::generic<required>,
    ph::generic<optional>,
    ph::leftover_t,
    ph::process<id>,
    ph::process<name>,
    ph::thread<id>,
    ph::thread<hex>,
    ph::thread<name>,
    ph::message_t,
    ph::severity<num>,
    ph::severity<user>,
    ph::timestamp<num>,
    ph::timestamp<user>
> token_t;

}  // namespace string
}  // namespace formatter
}  // namespace v1
}  // namespace blackhole
