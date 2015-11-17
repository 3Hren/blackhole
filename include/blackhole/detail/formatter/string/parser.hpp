#pragma once

#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>

#include "blackhole/detail/formatter/string/error.hpp"

namespace blackhole {
namespace detail {
namespace formatter {
namespace string {

namespace placeholder {

struct generic_t {
    std::string name;
    std::string spec;
};

struct message_t {
    std::string spec;
};

template<typename T>
struct severity {
    std::string spec;
};

struct numeric_severity_t {
    std::string spec;
};

struct timestamp_t {
    std::string pattern;
    std::string spec;
};

struct leftover_t {
    std::string name;
};

}  // namespace placeholder

struct literal_t {
    std::string value;
};

namespace ph = placeholder;

/// Helper named structs for making eye-candy code.
struct num;
struct user;

class parser_t {
public:
    typedef char char_type;
    typedef std::basic_string<char_type> string_type;

    typedef boost::variant<
        literal_t,
        ph::generic_t,
        ph::leftover_t,
        // ph::process<id>,
        // ph::process<name>,
        // ph::thread<id>,
        // ph::thread<hex>,
        // ph::thread<name>,
        ph::message_t,
        ph::severity<num>,
        ph::severity<user>,
        // ph::timestamp<num>,
        // ph::timestamp<user>,
        ph::timestamp_t
    > token_t;

private:
    enum class state_t {
        /// Undetermined state.
        unknown,
        /// Parsing literal.
        literal,
        /// Parsing placeholder.
        placeholder,
        /// Parser is broken.
        broken
    };

    state_t state;

    const std::string pattern;
    string_type::const_iterator pos;

public:
    explicit parser_t(std::string pattern);

    auto next() -> boost::optional<token_t>;

private:
    auto parse_unknown() -> boost::optional<token_t>;
    auto parse_literal() -> token_t;
    auto parse_placeholder() -> token_t;

    template<typename T>
    auto parse_spec(T token) -> token_t;
    auto parse_spec(placeholder::timestamp_t token) -> token_t;

    template<class Exception, class... Args>
    __attribute__((noreturn)) auto throw_(Args&&... args) -> void;
};

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace blackhole
