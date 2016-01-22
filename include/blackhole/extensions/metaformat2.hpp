#pragma once

#include <array>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>

#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {
inline namespace v1 {
namespace experimental {

class token_t {
    enum class type_t {
        literal,
        placeholder
    };

    string_view value;
    type_t type;

public:
    constexpr static auto literal(string_view value) -> token_t {
        return {value, type_t::literal};
    }

    constexpr static auto placeholder(string_view value) -> token_t {
        return {value, type_t::placeholder};
    }

    constexpr auto get() const noexcept -> const string_view& {
        return value;
    }

    constexpr auto is_literal() const noexcept -> bool {
        return type == type_t::literal;
    }

    constexpr auto operator==(const token_t& other) const noexcept -> bool {
        return value == other.value && type == other.type;
    }

private:
    constexpr token_t(string_view value, type_t type) :
        value(value),
        type(type)
    {}
};

class literal_t {
    /// The value can contain "{{" and "}}" sequences, which however will be correctly handled by
    /// the writer runtime.
    string_view value;

public:
    constexpr literal_t(string_view value) noexcept : value(value) {}

    constexpr auto get() const noexcept -> string_view {
        return value;
    }

    constexpr auto operator==(const literal_t& other) const noexcept -> bool {
        return value == other.value;
    }
};

class placeholder_t {
    string_view value;

public:
    constexpr placeholder_t(string_view value) noexcept : value(value) {}

    constexpr auto get() const noexcept -> string_view {
        return value;
    }

    constexpr auto operator==(const placeholder_t& other) const noexcept -> bool {
        return value == other.value;
    }
};


namespace detail {

constexpr auto parse_placeholder(string_view pattern) -> std::size_t {
    std::size_t id = 0;
    if (pattern[id] == '}') {
        return 1;
    }

    throw std::runtime_error("only \"{}\" is supported right now");
}

template<typename>
constexpr auto count(string_view pattern) -> std::size_t;

// Plan:
//  Given: "pattern with {} some {:d} placeholders"_pattern.
//   -> struct ::=
//      constexpr pattern() -> string_view;
//      constexpr literals() -> array<literal_t, N>;
//      constexpr placeholders() -> array<placeholder_t, M>;
//      format(stream&, args...);

/// Finds the number of literal expressions in the given pattern.
///
/// The pattern must be a valid logging expression, otherwise an exception will be thrown.
template<>
constexpr auto count<literal_t>(string_view pattern) -> std::size_t {
    std::size_t count = 0;

    std::size_t id = 0;
    while (true) {
        if (id == pattern.size()) {
            break;
        }

        // Possible placeholder begins, but also can be double "{{" mark.
        if (pattern[id] == '{') {
            if (id + 1 < pattern.size() && pattern[id + 1] == '{') {
                // Produces a single "{".
                id += 1;
            } else {
                // Placeholder begins.
                if (id != 0) {
                    ++count;
                }

                id += 1;
                id += parse_placeholder(pattern.substr(id));

                if (id == pattern.size()) {
                    return count;
                }
            }
        }

        id += 1;
    }

    return count + 1;
}

template<>
constexpr auto count<placeholder_t>(string_view pattern) -> std::size_t {
    std::size_t count = 0;

    std::size_t id = 0;
    while (true) {
        if (id == pattern.size()) {
            break;
        }

        // Possible placeholder begins, but also can be double "{{" mark.
        if (pattern[id] == '{') {
            ++id;

            if (id < pattern.size() && pattern[id] == '{') {
                // Produces a single "{".
                id += 1;
            } else {
                // Placeholder begins.
                id += parse_placeholder(pattern.substr(id));
                ++count;
            }
        } else {
            ++id;
        }
    }

    return count;
}

template<typename T>
struct display {
    template<typename W>
    static constexpr auto format(W& wr, const T& value) -> void {
        wr << value;
    }
};

class tokenizer_t {
    string_view pattern;

public:
    constexpr explicit tokenizer_t(string_view pattern) : pattern(pattern) {}

    constexpr auto count() const -> std::tuple<std::size_t, std::size_t> {
        std::size_t nliterals = 0;
        std::size_t nplaceholders = 0;

        std::size_t id = 0;
        std::size_t prev = 0;
        // Current literal view.

        while (true) {
            if (id == pattern.size()) {
                if (id != prev) {
                    ++nliterals;
                }
                break;
            }

            // Possible placeholder begins, but also can be double "{{" mark.
            if (pattern[id] == '{') {
                if (id + 1 < pattern.size() && pattern[id + 1] == '{') {
                    // Produces a single "{" and breaks the string.

                    ++id;
                    ++id;
                    prev = id;
                    ++nliterals;
                    continue;
                } else {
                    // Placeholder begins.
                    if (id != prev) {
                        ++nliterals;
                    }
                    ++id;
                    ++nplaceholders;
                    id += parse_placeholder(pattern.substr(id));
                    prev = id;
                    continue;
                }
            } else if (pattern[id] == '}') {
                if (id + 1 < pattern.size() && pattern[id + 1] == '}') {
                    // Produces a single "}" and breaks the string.
                    ++id;
                    ++id;
                    prev = id;
                    ++nliterals;
                    continue;
                } else {
                    throw std::invalid_argument("found unmatched } symbol");
                }
            }

            ++id;
        }

        return {nliterals, nplaceholders};
    }

    constexpr auto get(std::size_t n) const -> token_t {
        std::size_t nliterals = 0;
        std::size_t nplaceholders = 0;

        std::size_t id = 0;
        std::size_t prev = 0;
        // Current literal view.

        while (true) {
            if (id == pattern.size()) {
                if (id != prev) {
                    if (nliterals + nplaceholders == n) {
                        return token_t::literal(pattern.substr(prev));
                    }
                    ++nliterals;
                }
                break;
            }

            // Possible placeholder begins, but also can be double "{{" mark.
            if (pattern[id] == '{') {
                if (id + 1 < pattern.size() && pattern[id + 1] == '{') {
                    // Produces a single "{" and breaks the string.
                    if (nliterals + nplaceholders == n) {
                        return token_t::literal(pattern.substr(prev, id - prev + 1));
                    }
                    ++nliterals;

                    ++id;
                    ++id;
                    prev = id;
                    continue;
                } else {
                    // Placeholder begins.
                    if (id != prev) {
                        if (nliterals + nplaceholders == n) {
                            return token_t::literal(pattern.substr(prev, id - prev));
                        }
                        ++nliterals;
                    }

                    ++id;
                    auto nread = parse_placeholder(pattern.substr(id));
                    if (nliterals + nplaceholders == n) {
                        return token_t::placeholder(pattern.substr(id - 1, nread + 1));
                    }
                    ++nplaceholders;
                    id += nread;
                    prev = id;
                    continue;
                }
            } else if (pattern[id] == '}') {
                if (id + 1 < pattern.size() && pattern[id + 1] == '}') {
                    // Produces a single "}" and breaks the string.
                    if (nliterals + nplaceholders == n) {
                        return token_t::literal(pattern.substr(prev, id - prev + 1));
                    }
                    ++nliterals;

                    ++id;
                    ++id;
                    prev = id;
                    continue;
                } else {
                    throw std::invalid_argument("found unmatched } symbol");
                }
            }

            ++id;
        }

        throw std::out_of_range("out of range");
    }

    template<std::size_t N>
    constexpr auto tokens() const -> std::array<token_t, N> {
        return apply(std::make_index_sequence<N>());
    }

    template<std::size_t... I>
    constexpr auto apply(std::index_sequence<I...>) const -> std::array<token_t, sizeof...(I)> {
        return {{get(I)...}};
    }
};

template<std::size_t P, std::size_t N>
struct tokenizer {
    static constexpr auto L = N - P;

    string_view pattern_;
    std::array<token_t, N> tokens;

    constexpr explicit tokenizer(string_view pattern) :
        pattern_(pattern),
        tokens(tokenizer_t(pattern).tokens<N>())
    {}

    template<typename W, typename... Args>
    __attribute__((always_inline))
    constexpr auto format(W& wr, const Args&... args) const -> void {
        static_assert(sizeof...(Args) == P, "");
        fmt<0, sizeof...(Args)>(wr, args...);
    }

private:
    template<std::size_t I, std::size_t PP, typename W, typename T, typename... Args>
    __attribute__((always_inline))
    constexpr auto fmt(W& wr, const T& arg, const Args&... args) const ->
        typename std::enable_if<I < N>::type
    {
        if (tokens[I].is_literal()) {
            wr << fmt::StringRef(tokens[I].get().data(), tokens[I].get().size());
            fmt<I + 1, PP>(wr, arg, args...);
        } else {
            display<T>::format(wr, arg);
            fmt<I + 1, PP - 1>(wr, args...);
        }
    }

    template<std::size_t I, std::size_t PP, typename W, typename... Args>
    __attribute__((always_inline))
    constexpr auto fmt(W& wr, const Args&... args) const ->
        typename std::enable_if<I < N && PP == 0>::type
    {
        wr << fmt::StringRef(tokens[I].get().data(), tokens[I].get().size());
        fmt<I + 1, PP>(wr, args...);
    }

    template<std::size_t I, std::size_t PP, typename W, typename... Args>
    __attribute__((always_inline))
    constexpr auto fmt(W&, const Args&...) const ->
        typename std::enable_if<I >= N>::type
    {}
};

template<typename T>
constexpr auto get(string_view pattern, std::size_t n) -> T;

template<>
constexpr auto get<literal_t>(string_view pattern, std::size_t n) -> literal_t {
    std::size_t count = 0;

    std::size_t id = 0;
    std::size_t from = 0;
    while (true) {
        if (id == pattern.size()) {
            break;
        }

        // Possible placeholder begins, but also can be double "{{" mark.
        if (pattern[id] == '{') {
            if (id + 1 < pattern.size() && pattern[id + 1] == '{') {
                // Produces a single "{".
                id += 1;
            } else {
                // Placeholder begins.
                if (count == n) {
                    return {pattern.substr(from, id - from)};
                }

                if (id != 0) {
                    ++count;
                }

                id += 1;
                id += parse_placeholder(pattern.substr(id));
                from = id;
            }
        }

        id += 1;
    }

    return {pattern.substr(from)};
}

template<>
constexpr auto get<placeholder_t>(string_view pattern, std::size_t n) -> placeholder_t {
    std::size_t count = 0;

    std::size_t id = 0;
    std::size_t from = 0;
    while (true) {
        if (id == pattern.size()) {
            break;
        }

        // Possible placeholder begins, but also can be double "{{" mark.
        if (pattern[id] == '{') {
            ++id;

            if (id < pattern.size() && pattern[id] == '{') {
                // Produces a single "{".
                id += 1;
            } else {
                // Placeholder begins.
                const auto nread = parse_placeholder(pattern.substr(id));
                if (count == n) {
                    return pattern.substr(id - 1, nread + 1);
                }

                ++count;
                id += nread;
            }
        } else {
            ++id;
        }
    }

    if (n >= count) {
        throw std::out_of_range("out of range");
    }

    return {pattern.substr(from)};
}

template<typename T>
struct collect_traits {
    template<std::size_t... I>
    static constexpr auto apply(string_view pattern, std::index_sequence<I...>) ->
        std::array<T, sizeof...(I)>
    {
        return {get<T>(pattern, I)...};
    }
};

template<typename T, std::size_t N>
constexpr auto collect(string_view pattern) -> std::array<T, N> {
    return collect_traits<T>::apply(pattern, std::make_index_sequence<N>());
}

template<std::size_t L, std::size_t P>
class pattern {
    string_view pattern_;
    std::array<literal_t, L> literals_;
    std::array<placeholder_t, P> placeholders_;

public:
    constexpr explicit pattern(string_view pattern) :
        pattern_(pattern),
        literals_(collect<literal_t, L>(pattern)),
        placeholders_(collect<placeholder_t, P>(pattern))
    {
        if (count<literal_t>(pattern) != L) {
            throw std::invalid_argument("number of literals mismatch");
        }

        if (count<placeholder_t>(pattern) != P) {
            throw std::invalid_argument("number of placeholders mismatch");
        }
    }

    constexpr auto get() const noexcept -> const string_view& {
        return pattern_;
    }

    constexpr auto literals() const noexcept -> const std::array<literal_t, L>& {
        // TODO: Need to rewrite to squash "{{" and "}}" cases.
        return literals_;
    }

    constexpr auto placeholders() const noexcept -> const std::array<placeholder_t, P>& {
        return placeholders_;
    }

    template<typename W, typename... Args>
    constexpr auto format(W& wr, const Args&...) const ->
        typename std::enable_if<sizeof...(Args) == P>::type
    {
        // Always start with literal (even if it is empty).
        // Then chain `<< literal << placeholder;`
        // Always finish with literal (even if it is empty).
        //  make_index_sequence<sizeof...(Args) + 2>()
        //   (display<typename tuple_element<I, (decltype(tuple))>::type>::display(wr, get<I>(tuple)), ... );
        //  ^ Requires template tricks.
        // for (auto token : tokens) {
        //    if (token.literal()) { wr << token; fold<N + 1>(wr, arg, args...); }
        //      else { display<T>::format(wr, token, arg); fold<N + 1>(wr, args...); }
        // }
    }
};

}  // namespace detail

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
