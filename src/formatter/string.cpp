#include "blackhole/formatter/string.hpp"

#include <boost/type_traits/remove_cv.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>

#include <cppformat/format.h>

#include "blackhole/extensions/writer.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/formatter/string/parser.hpp"
#include "blackhole/detail/formatter/string/token.hpp"
#include "blackhole/detail/procname.hpp"

namespace blackhole {
namespace formatter {

namespace string = blackhole::detail::formatter::string;
namespace ph = string::ph;

using string::id;
using string::hex;
using string::num;
using string::name;
using string::user;
using string::required;
using string::optional;

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

class transform_visitor_t : public boost::static_visitor<string::token_t> {
    const options_t& options;

public:
    transform_visitor_t(const options_t& options) :
        options(options)
    {}

    auto operator()(const ph::generic<required>& token) const -> string::token_t {
        const auto it = options.find(token.name);

        if (it != options.end()) {
            const auto option = boost::get<option::optional_t>(it->second);

            return ph::generic<optional>(token, option.prefix, option.suffix);
        }

        return token;
    }

    template<typename T>
    auto operator()(const T& token) const -> string::token_t {
        return token;
    }
};

struct spec;
struct unspec;

template<typename Spec>
class view_visitor;

template<>
class view_visitor<spec> : public boost::static_visitor<> {
    writer_t& writer;
    const std::string& spec;

public:
    view_visitor(writer_t& writer, const std::string& spec) noexcept :
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

template<>
class view_visitor<unspec> : public boost::static_visitor<> {
    writer_t& writer;

public:
    view_visitor(writer_t& writer) noexcept :
        writer(writer)
    {}

    template<typename T>
    auto operator()(T value) const -> void {
        writer.inner << value;
    }

    auto operator()(const string_view& value) const -> void {
        writer.inner << fmt::StringRef(value.data(), value.size());
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

    auto operator()(const ph::process<id>& token) const -> void {
        writer.write(token.spec, record.pid());
    }

    auto operator()(const ph::process<name>& token) const -> void {
        writer.write(token.spec, detail::procname().data());
    }

    // TODO: ph::thread<id>.

    auto operator()(const ph::thread<hex>& token) const -> void {
        const auto tid = ::pthread_self();
#ifdef __linux__
        writer.write(token.spec, tid);
#elif __APPLE__
        writer.write(token.spec, reinterpret_cast<unsigned long>(tid));
#endif
    }

    // TODO: ph::thread<name>.

    auto operator()(const ph::severity<num>& token) const -> void {
        writer.write(token.spec, record.severity());
    }

    auto operator()(const ph::severity<user>& token) const -> void {
        sevmap(record.severity(), token.spec, writer);
    }

    auto operator()(const ph::timestamp<num>& token) const -> void {
        const auto timestamp = record.timestamp();
        const auto usec = std::chrono::duration_cast<
            std::chrono::microseconds
        >(timestamp.time_since_epoch()).count();

        writer.write(token.spec, usec);
    }

    auto operator()(const ph::timestamp<user>& token) const -> void {
        const auto timestamp = record.timestamp();
        const auto time = record_t::clock_type::to_time_t(timestamp);
        const auto usec = std::chrono::duration_cast<
            std::chrono::microseconds
        >(timestamp.time_since_epoch()).count() % 1000000;

        std::tm tm;
        ::gmtime_r(&time, &tm);

        fmt::MemoryWriter buffer;
        token.generator(buffer, tm, static_cast<std::uint64_t>(usec));
        writer.write(token.spec, fmt::StringRef(buffer.data(), buffer.size()));
    }

    auto operator()(const ph::generic<required>& token) const -> void {
        if (auto value = find(token.name)) {
            return value->apply(view_visitor<spec>(writer, token.spec));
        }

        throw std::logic_error("required attribute '" + token.name + "' not found");
    }

    auto operator()(const ph::generic<optional>& token) const -> void {
        if (auto value = find(token.name)) {
            writer.write(token.prefix);
            value->apply(view_visitor<spec>(writer, token.spec));
            writer.write(token.suffix);
        }
    }

    auto operator()(const ph::leftover_t& token) const -> void {
        bool first = true;
        const view_visitor<unspec> visitor(writer);

        for (const auto& attributes : record.attributes()) {
            for (const auto& attribute : attributes.get()) {
                if (first) {
                    first = false;
                } else {
                    writer.inner << ", ";
                }

                writer.inner << fmt::StringRef(attribute.first.data(), attribute.first.size())
                    << ": ";
                attribute.second.apply(visitor);
            }
        }
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

static auto tokenize(const std::string& pattern, const options_t& options) -> std::vector<token_t> {
    std::vector<token_t> tokens;

    for (const auto& reserved : {"process", "thread", "message", "severity", "timestamp"}) {
        if (options.count(reserved) != 0) {
            throw std::logic_error("placeholder '" + std::string(reserved) +
                "' is reserved and can not be configured");
        }
    }

    string::parser_t parser(pattern);
    while (auto token = parser.next()) {
        tokens.emplace_back(boost::apply_visitor(transform_visitor_t(options), token.get()));
    }

    return tokens;
}

}  // namespace

string_t::string_t(std::string pattern, const options_t& options) :
    pattern(std::move(pattern)),
    sevmap([](int severity, const std::string& spec, writer_t& writer) {
        writer.write(spec, severity);
    }),
    tokens(tokenize(this->pattern, options))
{}

string_t::string_t(std::string pattern, severity_map sevmap, const options_t& options) :
    pattern(std::move(pattern)),
    sevmap(std::move(sevmap)),
    tokens(tokenize(this->pattern, options))
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
