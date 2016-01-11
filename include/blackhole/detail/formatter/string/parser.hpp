#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/optional/optional.hpp>

#include "blackhole/detail/formatter/string/error.hpp"
#include "blackhole/detail/formatter/string/token.hpp"

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace formatter {
namespace string {

class factory_t;

class parser_t {
public:
    typedef char char_type;
    typedef std::basic_string<char_type> string_type;

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

    std::unordered_map<std::string, std::shared_ptr<factory_t>> factories;

public:
    explicit parser_t(std::string pattern);

    auto next() -> boost::optional<token_t>;

private:
    auto parse_spec() -> std::string;
    auto parse_unknown() -> boost::optional<token_t>;
    auto parse_literal() -> literal_t;
    auto parse_placeholder() -> token_t;

    /// Returns `true` on exact match with the given range from the current position.
    ///
    /// The given range may be larger than `std::distance(pos, std::end(pattern))`.
    template<typename Range>
    auto exact(const Range& range) const -> bool;

    /// Returns `true` on exact match with the given range and position.
    ///
    /// The given range may be larger than `std::distance(pos, std::end(pattern))`.
    ///
    /// \overload
    template<typename Range>
    auto exact(const_iterator pos, const Range& range) const -> bool;

    /// Marks the parser as broken and throws an exception
    template<class Exception, class... Args>
    __attribute__((noreturn)) auto throw_(Args&&... args) -> void;
};

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace v1
}  // namespace blackhole
