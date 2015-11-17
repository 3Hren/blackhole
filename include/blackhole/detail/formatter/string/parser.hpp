#pragma once

#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>

#include "blackhole/detail/formatter/string/error.hpp"

namespace blackhole {
namespace detail {
namespace formatter {
namespace string {

struct literal_t {
    std::string value;
};

struct placeholder_t {
    std::string name;
    std::string spec;
};

struct message_t {
    std::string spec;
};

struct severity_t {
    std::string spec;
};

struct timestamp_t {
    std::string pattern;
    std::string spec;
};

class parser_t {
public:
    typedef boost::variant<
        literal_t,
        message_t,
        severity_t,
        timestamp_t,
        placeholder_t
    > token_t;

private:
    enum class state_t {
        /// Undetermined state.
        whatever,
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
    auto begin() const -> iterator_type;
    auto end() const -> iterator_type;

    auto parse_unknown() -> boost::optional<token_t>;
    auto parse_literal() -> token_t;
    auto parse_placeholder() -> token_t;

    template<typename T>
    auto parse_spec(T token) -> token_t;
    auto parse_timestamp(timestamp_t token) -> token_t;

    template<class Exception, class... Args>
    __attribute__((noreturn)) auto throw_(Args&&... args) -> void;
};

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace blackhole
