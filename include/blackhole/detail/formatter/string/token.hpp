#pragma once

#include <string>

#include <boost/variant/variant_fwd.hpp>

namespace blackhole {
namespace detail {
namespace formatter {
namespace string {

/// Helper named structs for making eye-candy code.
struct id;
struct hex;
struct num;
struct name;
struct user;

/// Represents string literal.
struct literal_t {
    std::string value;
};

namespace placeholder {

struct generic_t {
    std::string name;
    std::string spec;
    std::string prefix;
    std::string suffix;

    generic_t(std::string name);
    generic_t(std::string name, std::string spec);
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

    timestamp();
    timestamp(std::string pattern, std::string spec);
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

struct leftover_t {
    std::string name;

    bool unique;
    std::string prefix;
    std::string suffix;
    std::string pattern;
    std::string separator;

    leftover_t();
    leftover_t(std::string name);
};

}  // namespace placeholder

namespace ph = placeholder;

typedef boost::variant<
    literal_t,
    ph::generic_t,
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
}  // namespace detail
}  // namespace blackhole
