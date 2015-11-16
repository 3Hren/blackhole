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

typedef boost::variant<
    literal_t,
    placeholder_t
> token_t;

class parser_t {
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

    const std::string pattern;

    state_t state;
    std::string::const_iterator pos;

public:
    explicit parser_t(std::string pattern);

    auto next() -> boost::optional<token_t>;

private:
    auto begin() const -> std::string::const_iterator;
    auto end() const -> std::string::const_iterator;

    auto parse_unknown() -> boost::optional<token_t>;
    auto parse_literal() -> token_t;
    auto parse_placeholder() -> token_t;
    auto parse_spec(placeholder_t placeholder) -> token_t;

    template<class Exception, class... Args>
    __attribute__((noreturn)) auto throw_(Args&&... args) -> void;
};

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace blackhole
