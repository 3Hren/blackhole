#include "blackhole/formatter/string.hpp"

#include <boost/variant/variant.hpp>

#include <cppformat/format.h>

#include "blackhole/extensions/writer.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/formatter/string/parser.hpp"
#include "blackhole/detail/formatter/string/token.hpp"

namespace blackhole {
namespace formatter {

namespace string = blackhole::detail::formatter::string;
namespace ph = string::ph;

using string::num;
using string::user;

using string::literal_t;

class token_t {
    string::token_t inner;

public:
    token_t(string::token_t inner) :
        inner(std::move(inner))
    {}

    auto operator*() const noexcept -> const string::token_t& {
        return inner;
    }
};

namespace {

class view_visitor_t : public boost::static_visitor<> {
    writer_t& writer;
    const std::string& spec;

public:
    view_visitor_t(writer_t& writer, const std::string& spec) noexcept :
        writer(writer),
        spec(spec)
    {}

    template<typename T>
    auto operator()(T value) const -> void {
        writer.write(spec, value);
    }

    auto operator()(const string_view& value) const -> void {
        writer.write(spec, value.data());
    }
};

class visitor_t : public boost::static_visitor<> {
    writer_t& writer;
    const record_t& record;
    const severity_map& sevmap;

public:
    visitor_t(writer_t& writer, const record_t& record, const severity_map& sevmap) noexcept :
        writer(writer),
        record(record),
        sevmap(sevmap)
    {}

    auto operator()(const literal_t& token) const -> void {
        writer.inner << token.value;
    }

    auto operator()(const ph::message_t& token) const -> void {
        writer.write(token.spec, record.formatted().data());
    }

    auto operator()(const ph::severity<num>& token) const -> void {
        writer.write(token.spec, record.severity());
    }

    auto operator()(const ph::severity<user>& token) const -> void {
        sevmap(record.severity(), token.spec, writer);
    }

    auto operator()(const ph::generic_t& token) const -> void {
        if (auto value = find(token.name)) {
            value->apply(view_visitor_t(writer, token.spec));
            return;
        }

        std::terminate();
    }

    template<typename T>
    auto operator()(const T& token) const -> void {
        std::terminate();
    }

private:
    auto find(const std::string& name) const -> boost::optional<attribute::view_t> {
        for (const auto& attributes : record.attributes()) {
            for (const auto& attribute : attributes.get()) {
                if (attribute.first == name) {
                    return attribute.second;
                }
            }
        }

        return boost::none;
    }
};

static auto tokenize(const std::string& pattern) -> std::vector<token_t> {
    std::vector<token_t> tokens;

    string::parser_t parser(pattern);
    while (auto token = parser.next()) {
        tokens.emplace_back(token.get());
    }

    return tokens;
}

}  // namespace

string_t::string_t(std::string pattern, options_t options) :
    pattern(std::move(pattern)),
    sevmap([](int severity, const std::string& spec, writer_t& writer) {
        writer.write(spec, severity);
    }),
    tokens(tokenize(this->pattern))
{}

string_t::string_t(std::string pattern, severity_map sevmap, options_t options) :
    pattern(std::move(pattern)),
    sevmap(std::move(sevmap)),
    tokens(tokenize(this->pattern))
{}

string_t::~string_t() {}

auto
string_t::format(const record_t& record, writer_t& writer) -> void {
    const visitor_t visitor(writer, record, sevmap);

    for (const auto& token : tokens) {
        boost::apply_visitor(visitor, *token);
    }
}

}  // namespace formatter
}  // namespace blackhole
