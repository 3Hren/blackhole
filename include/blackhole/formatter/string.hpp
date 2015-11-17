#pragma once

#include <map>
#include <string>
#include <vector>

#include <boost/variant/variant.hpp>

#include "blackhole/formatter.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/formatter/string/parser.hpp"

namespace blackhole {
namespace formatter {

using detail::formatter::string::parser_t;

using namespace detail::formatter::string;

namespace option {

struct optional_t {
    std::string prefix;
    std::string suffix;
};

struct remaining_t {
    bool unique;
    std::string prefix;
    std::string suffix;
    std::string pattern;
    std::string separator;
};

}  // namespace option

typedef boost::variant<
    option::optional_t,
    option::remaining_t
> option_t;

typedef std::function<void(int, writer_t&)> severity_function;

class token_visitor_t : public boost::static_visitor<> {
    const record_t& record;
    // const severity_function fn;
    writer_t& writer;

public:
    token_visitor_t(const record_t& record, writer_t& writer) :
        record(record),
        writer(writer)
    {}

    auto operator()(const literal_t& token) -> void {
        writer.inner << token.value;
    }

    auto operator()(const message_t& token) -> void {
        writer.write("{" + token.spec + "}", record.formatted());
    }

    // severity_t,
    // timestamp_t,
    // placeholder_t,
    // placeholder::leftover_t

    template<typename T>
    auto operator()(const T& token) -> void {
        throw std::runtime_error("unimplemented");
    }
};

class string_t : formatter_t {
    const std::string pattern;
    // const severity_function fn;
    const std::vector<parser_t::token_t> tokens;

public:
    string_t(std::string pattern, std::map<std::string, option_t> options = {}) :
        pattern(std::move(pattern)),
        // fn(std::move(fn)),
        tokens(tokenize(this->pattern))
    {}
    // pattern -> [token].
    // token.each -> find options[token.name] -> double match & attach options.

    auto format(const record_t& record, writer_t& writer) -> void {
        token_visitor_t visitor(record, writer);

        for (const auto& token : tokens) {
            boost::apply_visitor(visitor, token);
        }
    }

private:
    static auto tokenize(const std::string& pattern) -> std::vector<parser_t::token_t> {
        std::vector<parser_t::token_t> tokens;

        parser_t parser(pattern);
        while (auto token = parser.next()) {
            tokens.emplace_back(token.get());
        }

        return tokens;
    }
};

}  // namespace formatter
}  // namespace blackhole
