#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>

#include "blackhole/detail/formatter/string/error.hpp"

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

    generic_t(std::string name) noexcept :
        name(std::move(name)),
        spec("{}")
    {}

    generic_t(std::string name, std::string spec) noexcept :
        name(std::move(name)),
        spec(std::move(spec))
    {}
};

struct message_t {
    std::string spec;

    message_t() noexcept : spec("{}") {}
    message_t(std::string spec) noexcept : spec(std::move(spec)) {}
};

template<typename T>
struct severity {
    std::string spec;

    severity() noexcept : spec("{}") {}
    severity(std::string spec) noexcept : spec(std::move(spec)) {}
};

template<typename T>
struct timestamp;

template<>
struct timestamp<num> {
    std::string spec;

    timestamp() noexcept : spec("{}") {}
    timestamp(std::string spec) noexcept : spec(std::move(spec)) {}
};

template<>
struct timestamp<user> {
    std::string pattern;
    std::string spec;

    timestamp() noexcept :
        pattern(),
        spec("{}")
    {}

    timestamp(std::string pattern, std::string spec) noexcept :
        pattern(std::move(pattern)),
        spec(std::move(spec))
    {}
};

template<typename T>
struct process {
    std::string spec;

    process() noexcept : spec("{}") {}
    process(std::string spec) noexcept : spec(std::move(spec)) {}
};

struct leftover_t {
    std::string name;
};

}  // namespace placeholder

struct literal_t {
    std::string value;
};

namespace ph = placeholder;

struct spec_factory_t;

template<typename T>
struct default_spec_factory;

template<typename T>
struct spec_factory;

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
