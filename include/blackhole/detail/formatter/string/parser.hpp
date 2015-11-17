#pragma once

#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>

#include "blackhole/detail/formatter/string/error.hpp"

namespace blackhole {
namespace detail {
namespace formatter {
namespace string {

namespace placeholder {

struct common_t {
    std::string name;
    std::string spec;
};

struct message_t {
    std::string spec;
};

struct severity_t {
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

class parser_t {
public:
    typedef boost::variant<
        literal_t,
        placeholder::common_t,
        placeholder::message_t,
        placeholder::severity_t,
        placeholder::severity_t,
        placeholder::numeric_severity_t,
        placeholder::timestamp_t,
        placeholder::leftover_t
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

    typedef std::string::const_iterator iterator_type;

    const std::string pattern;

    state_t state;
    iterator_type pos;

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
