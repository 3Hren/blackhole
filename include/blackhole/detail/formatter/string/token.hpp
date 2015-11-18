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

    generic_t(std::string name) noexcept;
    generic_t(std::string name, std::string spec) noexcept;
};

struct message_t {
    std::string spec;

    message_t() noexcept;
    message_t(std::string spec) noexcept;
};

template<typename T>
struct severity {
    std::string spec;

    severity() noexcept;
    severity(std::string spec) noexcept;
};

template<typename T>
struct timestamp;

template<>
struct timestamp<num> {
    std::string spec;

    timestamp() noexcept;
    timestamp(std::string spec) noexcept;
};

template<>
struct timestamp<user> {
    std::string pattern;
    std::string spec;

    timestamp() noexcept;
    timestamp(std::string pattern, std::string spec) noexcept;
};

template<typename T>
struct process {
    std::string spec;

    process() noexcept;
    process(std::string spec) noexcept;
};

template<typename T>
struct thread {
    std::string spec;

    thread() noexcept;
    thread(std::string spec) noexcept;
};

struct leftover_t {
    std::string name;
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
