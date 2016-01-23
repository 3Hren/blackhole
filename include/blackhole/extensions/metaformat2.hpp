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

    constexpr auto operator!=(const token_t& other) const noexcept -> bool {
        return !(*this == other);
    }

private:
    constexpr token_t(string_view value, type_t type) :
        value(value),
        type(type)
    {}
};

namespace detail {

constexpr auto parse_placeholder(string_view pattern) -> std::size_t {
    std::size_t id = 0;
    if (pattern[id] == '}') {
        return 1;
    }

    throw std::runtime_error("only \"{}\" is supported right now");
}

template<typename T>
struct display {
    template<typename W>
    static constexpr auto format(W& wr, const T& value) -> void {
        wr << value;
    }
};

class tokenizer_t {
public:
    struct count_t {
        std::tuple<std::size_t, std::size_t> value;

        constexpr auto tokens() const noexcept -> std::size_t {
            return literals() + placeholders();
        }

        constexpr auto literals() const noexcept -> std::size_t {
            return std::get<0>(value);
        }

        constexpr auto placeholders() const noexcept -> std::size_t {
            return std::get<1>(value);
        }
    };

private:
    string_view pattern;

public:
    constexpr explicit tokenizer_t(string_view pattern) noexcept : pattern(pattern) {}

    constexpr auto count() const -> count_t {
        std::size_t nliterals = 0;
        std::size_t nplaceholders = 0;

        std::size_t id = 0;
        std::size_t prev = 0;

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

        return {{nliterals, nplaceholders}};
    }

    constexpr auto literal(std::size_t id) const -> string_view {
        std::size_t counter = 0;
        std::size_t hits = 0;

        while (true) {
            const auto token = get(counter);

            if (token.is_literal()) {
                if (hits == id) {
                    return token.get();
                }

                ++hits;
            }

            ++counter;
        }
    }

    constexpr auto placeholder(std::size_t id) const -> string_view {
        std::size_t counter = 0;
        std::size_t hits = 0;

        while (true) {
            const auto token = get(counter);

            if (!token.is_literal()) {
                if (hits == id) {
                    return token.get();
                }

                ++hits;
            }

            ++counter;
        }
    }

    constexpr auto get(std::size_t n) const -> token_t {
        std::size_t nliterals = 0;
        std::size_t nplaceholders = 0;

        std::size_t id = 0;
        std::size_t prev = 0;

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

    template<std::size_t N, std::size_t T>
    constexpr auto literals() const -> std::array<string_view, N> {
        std::size_t id = 0;
        std::array<string_view, N> result;

        for (auto token : tokens<T>()) {
            if (token.is_literal()) {
                result[id++] = token.get();
            }
        }

        return result;
    }

    template<std::size_t N, std::size_t T>
    constexpr auto placeholders() const -> std::array<string_view, N> {
        std::size_t id = 0;
        std::array<string_view, N> result;

        for (auto token : tokens<T>()) {
            if (!token.is_literal()) {
                result[id++] = token.get();
            }
        }

        return result;
    }

    template<std::size_t... I>
    constexpr auto apply(std::index_sequence<I...>) const -> std::array<token_t, sizeof...(I)> {
        return {{get(I)...}};
    }
};

/// \warning requires precise placeholders and total tokens count, otherwise the behavior is
///     undefined, probably won't compile.
template<std::size_t P, std::size_t N>
struct tokenizer {
    static constexpr auto L = N - P;

    string_view pattern;
    std::array<token_t, N> tokens;

    constexpr explicit tokenizer(string_view pattern) :
        pattern(pattern),
        tokens(tokenizer_t(pattern).tokens<N>())
    {}

    constexpr auto unparsed() const noexcept -> string_view {
        return pattern;
    }

    template<typename W, typename... Args>
    __attribute__((always_inline))
    constexpr auto format(W& wr, const Args&... args) const -> void {
        // TODO: Check arguments count mismatch: static_assert(sizeof...(Args) == P, "");
        // TODO: Check arguments type mismatch based on exceptions.
        fmt<0, sizeof...(Args)>(wr, args...);
    }

private:
    /// \note that it's important to always inline this method, because then constexpr magic may
    ///     result in compile-time branch elimination.
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

    // Terminal case when the current index is out of the given token range.
    template<std::size_t I, std::size_t PP, typename W, typename... Args>
    __attribute__((always_inline))
    constexpr auto fmt(W&, const Args&...) const ->
        typename std::enable_if<I >= N>::type
    {}
};

}  // namespace detail

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
