#include "blackhole/formatter/string.hpp"

#include <array>

#include <boost/type_traits/remove_cv.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>

#include <cppformat/format.h>

#include "blackhole/config.hpp"
#include "blackhole/config/monadic.hpp"
#include "blackhole/extensions/writer.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/attribute.hpp"
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

namespace {

typedef fmt::StringRef string_ref;

}  // namespace

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

    auto operator()(const ph::leftover_t& token) const -> string::token_t {
        const auto it = options.find(token.name);

        if (it != options.end()) {
            const auto option = boost::get<option::leftover_t>(it->second);

            return ph::leftover_t(token.name, option.unique, option.prefix, option.suffix,
                option.pattern, option.separator);
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

    auto operator()(std::nullptr_t) const -> void {
        writer.write(spec, "none");
    }

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
    fmt::MemoryWriter& writer;

public:
    view_visitor(fmt::MemoryWriter& writer) noexcept :
        writer(writer)
    {}

    auto operator()(std::nullptr_t) const -> void {
        writer << "none";
    }

    template<typename T>
    auto operator()(T value) const -> void {
        writer << value;
    }

    auto operator()(const string_view& value) const -> void {
        writer << string_ref(value.data(), value.size());
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
        const auto& value = record.formatted();
        writer.write(token.spec, string_ref(value.data(), value.size()));
    }

    auto operator()(const ph::process<id>& token) const -> void {
        writer.write(token.spec, record.pid());
    }

    auto operator()(const ph::process<name>& token) const -> void {
        writer.write(token.spec, detail::procname().data());
    }

    auto operator()(const ph::thread<id>& token) const -> void {
        throw std::runtime_error("{thread:d} placeholder is not implemented yet");
    }

    auto operator()(const ph::thread<hex>& token) const -> void {
#ifdef __linux__
        writer.write(token.spec, record.tid());
#elif __APPLE__
        writer.write(token.spec, reinterpret_cast<unsigned long>(record.tid()));
#endif
    }

    auto operator()(const ph::thread<name>& token) const -> void {
        std::array<char, 16> buffer;
        const auto rc = ::pthread_getname_np(record.tid(), buffer.data(), buffer.size());

        if (rc == 0) {
            writer.write(token.spec, buffer.data());
        } else {
            writer.write(token.spec, "<unnamed>");
        }
    }

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
        writer.write(token.spec, string_ref(buffer.data(), buffer.size()));
    }

    auto operator()(const ph::generic<required>& token) const -> void {
        if (auto value = find(token.name)) {
            return boost::apply_visitor(view_visitor<spec>(writer, token.spec), value->inner().value);
        }

        throw std::logic_error("required attribute '" + token.name + "' not found");
    }

    auto operator()(const ph::generic<optional>& token) const -> void {
        if (auto value = find(token.name)) {
            writer.write(token.prefix);
            boost::apply_visitor(view_visitor<spec>(writer, token.spec), value->inner().value);
            writer.write(token.suffix);
        }
    }

    auto operator()(const ph::leftover_t& token) const -> void {
        bool first = true;
        fmt::MemoryWriter kv;
        const view_visitor<unspec> visitor(kv);

        for (const auto& attributes : record.attributes()) {
            for (const auto& attribute : attributes.get()) {
                if (first) {
                    first = false;
                    writer.inner << token.prefix;
                } else {
                    writer.inner << token.separator;
                }

                // TODO: To correctly implement kv patterns we need a visitor with parameters. Or
                // attribute (pair) type, instead of that `std::pair`.
                kv << string_ref(attribute.first.data(), attribute.first.size()) << ": ";
                boost::apply_visitor(visitor, attribute.second.inner().value);

                writer.inner << string_ref(kv.data(), kv.size());

                kv.clear();
            }
        }

        if (!first) {
            writer.inner << token.suffix;
        }
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

string_t::string_t(string_t&& other) = default;

string_t::~string_t() {}

auto
string_t::format(const record_t& record, writer_t& writer) -> void {
    const visitor_t visitor(writer, record, sevmap);

    for (const auto& token : tokens) {
        boost::apply_visitor(visitor, *token);
    }
}

}  // namespace formatter

auto
factory<formatter::string_t>::type() -> const char* {
    return "string";
}

auto
factory<formatter::string_t>::from(const config_t& config) -> formatter::string_t {
    auto pattern = config["pattern"]->to_string();

    if (auto mapping = config["sevmap"]) {
        std::vector<std::string> sevmap;
        mapping->each([&](const config_t& config) {
            sevmap.emplace_back(config.to_string());
        });

        auto fn = [=](std::size_t severity, const std::string& spec, writer_t& writer) {
            if (severity < sevmap.size()) {
                writer.write(spec, sevmap[severity]);
            } else {
                writer.write(spec, severity);
            }
        };

        return formatter::string_t(std::move(pattern), std::move(fn));
    }

    return formatter::string_t(std::move(pattern));
}

}  // namespace blackhole
