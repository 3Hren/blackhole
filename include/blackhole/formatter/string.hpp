#pragma once

#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "blackhole/extensions/writer.hpp"
#include "blackhole/formatter.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/formatter/string/parser.hpp"
#include "blackhole/detail/formatter/string/generator.hpp"

namespace blackhole {
namespace formatter {

namespace string = detail::formatter::string;

class string_t : public formatter_t {
    const bool unique;
    const std::string pattern;
    const std::vector<string::token_t> tokens;
    string::severity_formatter sfn;
    string::timestamp_formatter tfn;

public:
    string_t(const std::string& pattern) :
        unique(true),
        pattern(pattern),
        tokens(tokenize(pattern)),
        sfn([](int value, writer_t& writer) {
            writer.inner << value;
        }),
        tfn([](const record_t::time_point& value, writer_t& writer) {
            writer.inner << std::chrono::duration_cast<std::chrono::microseconds>(value.time_since_epoch()).count();
        })
    {}

    string_t(const std::string& pattern, string::severity_formatter sfn, string::timestamp_formatter tfn) :
        unique(true),
        pattern(pattern),
        tokens(tokenize(pattern)),
        sfn(std::move(sfn)),
        tfn(std::move(tfn))
    {}

    auto format(const record_t& record, writer_t& writer) -> void {
        try {
            string::visitor_t visitor(record, writer, unique);
            visitor.sfn = sfn;
            visitor.tfn = tfn;

            for (const auto& token : tokens) {
                boost::apply_visitor(visitor, token);
            }
        } catch (const std::exception& err) {
            throw std::runtime_error("bad format string '" + pattern + "': " + err.what());
        }
    }

private:
    static
    std::vector<string::token_t>
    tokenize(const std::string& pattern) {
        std::vector<string::token_t> tokens;

        string::parser_t parser(pattern);
        while (auto token = parser.next()) {
            tokens.push_back(token.get());
        }

        return tokens;
    }
};

}  // namespace formatter
}  // namespace blackhole
