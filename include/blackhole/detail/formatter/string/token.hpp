#pragma once

#include <string>

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

struct leftover_t {
    std::string name;
};

}  // namespace placeholder

struct literal_t {
    std::string value;
};

namespace ph = placeholder;

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace blackhole
