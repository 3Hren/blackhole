#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>

#include "blackhole/detail/formatter/string/error.hpp"
#include "blackhole/detail/formatter/string/token.hpp"

namespace blackhole {
namespace detail {
namespace formatter {
namespace string {

struct spec_factory_t;

class parser_t {
public:
    typedef char char_type;
    typedef std::basic_string<char_type> string_type;

    typedef boost::variant<
        literal_t,
        ph::generic_t,
        ph::leftover_t,
        ph::process<id>,
        ph::process<name>,
        // ph::thread<id>,
        // ph::thread<hex>,
        // ph::thread<name>,
        ph::message_t,
        ph::severity<num>,
        ph::severity<user>,
        ph::timestamp<num>,
        ph::timestamp<user>
    > token_t;

private:
    typedef string_type::const_iterator const_iterator;

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
    const_iterator pos;

    std::unordered_map<std::string, std::shared_ptr<spec_factory_t>> factories;

public:
    explicit parser_t(std::string pattern);

    auto next() -> boost::optional<token_t>;

private:
    auto parse_unknown() -> boost::optional<token_t>;
    auto parse_literal() -> literal_t;
    auto parse_placeholder() -> token_t;

    auto parse_spec() -> std::string;

    /// Returns `true` on exact match with the given range from the current position.
    ///
    /// The given range may be larger than `std::distance(pos, std::end(pattern))`.
    template<typename Range>
    auto exact(const Range& range) const -> bool;

    template<typename Range>
    auto exact(const_iterator pos, const Range& range) const -> bool;

    template<class Exception, class... Args>
    __attribute__((noreturn)) auto throw_(Args&&... args) -> void;
};

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace blackhole
