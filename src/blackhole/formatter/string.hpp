#pragma once

#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "blackhole/config.hpp"

#include "blackhole/error.hpp"
#include "blackhole/detail/stream/stream.hpp"
#include "blackhole/formatter/base.hpp"
#include "blackhole/formatter/map/value.hpp"
#include "blackhole/formatter/string/config.hpp"
#include "blackhole/formatter/string/generator.hpp"
#include "blackhole/formatter/string/parser.hpp"
#include "blackhole/record.hpp"
#include "blackhole/repository/factory/traits.hpp"

BLACKHOLE_BEG_NS

namespace formatter {

class string_t : public base_t {
public:
    typedef string::config_t config_type;

    static const char* name() {
        return "string";
    }

private:
    const std::string pattern;
    bool filter;
    const std::vector<string::token_t> tokens;

public:
    string_t(const std::string& pattern) :
        pattern(pattern),
        filter(true),
        tokens(tokenize(pattern))
    {}

    string_t(const config_type& config) :
        pattern(config.pattern),
        filter(config.filter),
        tokens(tokenize(config.pattern))
    {}

    std::string format(const record_t& record) const {
        std::string buffer;
        stickystream_t stream;
        stream.attach(buffer);
        try {
            string::visitor_t visitor(stream, mapper, record.attributes(), filter);
            for (auto it = tokens.begin(); it != tokens.end(); ++it) {
                boost::apply_visitor(visitor, *it);
                stream.flush();
            }
            return buffer;
        } catch (const error_t& err) {
            throw error_t("bad format string '%s': %s", pattern, err.what());
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

} // namespace formatter

template<>
struct factory_traits<formatter::string_t> {
    typedef formatter::string_t::config_type config_type;

    static void map_config(const aux::extractor<formatter::string_t>& ex, config_type& cfg) {
        ex["pattern"].to(cfg.pattern);
    }
};

BLACKHOLE_END_NS
