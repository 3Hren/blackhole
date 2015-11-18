#include "blackhole/formatter/string.hpp"

#include <boost/variant/variant.hpp>
#include <cppformat/format.h>

#include "blackhole/extensions/writer.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/formatter/string/parser.hpp"
#include "blackhole/detail/formatter/string/token.hpp"
#include "blackhole/detail/unimplemented.hpp"

namespace blackhole {
namespace formatter {

namespace string = blackhole::detail::formatter::string;
namespace ph = string::ph;

using string::num;
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

    template<typename T>
    auto operator()(const T& token) const -> void {
        BLACKHOLE_UNIMPLEMENTED();
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
    tokens(tokenize(this->pattern))
{}

string_t::string_t(std::string pattern, severity_map sevmap, options_t options) {}

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
