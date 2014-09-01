#pragma once

#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "blackhole/error.hpp"
#include "blackhole/detail/stream/stream.hpp"
#include "blackhole/formatter/base.hpp"
#include "blackhole/formatter/map/value.hpp"
#include "blackhole/formatter/string/config.hpp"
#include "blackhole/formatter/string/parser.hpp"
#include "blackhole/record.hpp"
#include "blackhole/repository/factory/traits.hpp"

namespace blackhole {

namespace formatter {

class variadic_visitor_t : public boost::static_visitor<> {
    std::ostringstream& stream;

public:
    variadic_visitor_t(std::ostringstream& stream) :
        stream(stream)
    {}

    template<typename T>
    void operator()(const T& value) const {
        stream << value;
    }

    void operator()(const std::string& value) const {
        stream << "'" << value << "'";
    }
};


class visitor_t : public boost::static_visitor<> {
    blackhole::aux::attachable_ostringstream& stream;
    const mapping::value_t& mapper;
    const attribute::set_view_t& view;

public:
    visitor_t(blackhole::aux::attachable_ostringstream& stream,
              const mapping::value_t& mapper,
              const attribute::set_view_t& view) :
        stream(stream),
        mapper(mapper),
        view(view)
    {}

    void operator()(const string::literal_t& literal) {
        stream.rdbuf()->storage()->append(literal.value);
    }

    void operator()(const string::placeholder::required_t& ph) {
        if (auto attribute = view.find(ph.name)) {
            mapper(stream, ph.name, attribute->value);
            return;
        }

        throw error_t("required attribute '%s' was not provided", ph.name);
    }

    void operator()(const string::placeholder::optional_t& ph) {
        if (auto attribute = view.find(ph.name)) {
            stream.rdbuf()->storage()->append(ph.prefix);
            mapper(stream, ph.name, attribute->value);
            stream.rdbuf()->storage()->append(ph.suffix);
        }
    }

    void operator()(const string::placeholder::variadic_t& /*ph*/) {
        std::vector<std::string> passed;
        passed.reserve(view.upper_size());

        for (auto it = view.begin(); it != view.end(); ++it) {
            const std::string& name = it->first;
            const attribute_t& attribute = it->second;

            //!@todo: This code needs some optimization love.
            std::ostringstream stream;
            stream << "'" << name << "': ";
            variadic_visitor_t visitor(stream);
            boost::apply_visitor(visitor, attribute.value);
            passed.push_back(stream.str());
        }

        stream.rdbuf()->storage()->append(boost::algorithm::join(passed, ", "));
    }
};

class string_t : public base_t {
    const std::string pattern;
    const std::vector<string::token_t> tokens;

public:
    typedef string::config_t config_type;

    static const char* name() {
        return "string";
    }

    string_t(const std::string& pattern) :
        pattern(pattern),
        tokens(tokenize(pattern))
    {}

    string_t(const config_type& config) :
        pattern(config.pattern),
        tokens(tokenize(config.pattern))
    {}

    std::string format(const record_t& record) const {
        std::string buffer;
        blackhole::aux::attachable_ostringstream stream;
        stream.attach(buffer);
        try {
            visitor_t visitor(stream, mapper, record.attributes());
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

} // namespace blackhole
