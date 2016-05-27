#include "blackhole/formatter/string.hpp"

#include <array>

#include <boost/type_traits/remove_cv.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/extensions/format.hpp"
#include "blackhole/extensions/writer.hpp"
#include "blackhole/formatter.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/attribute.hpp"
#include "blackhole/detail/formatter/string/parser.hpp"
#include "blackhole/detail/formatter/string/token.hpp"
#include "blackhole/detail/memory.hpp"
#include "blackhole/detail/procname.hpp"
#include "blackhole/detail/util/deleter.hpp"

// TODO: Optional attributes.
// Optional placeholders allows to nicely format some patterns where there are non-reserved
// attributes used and its presents is undetermined. Unlike required placeholders it does not throw
// an exception if there are no attribute with the given name in the set.
// Also it provides additional functionality with optional surrounding a present attribute with
// some prefix and suffix.
namespace blackhole {
inline namespace v1 {
namespace formatter {

namespace {

namespace string = blackhole::detail::formatter::string;

namespace ph = string::ph;

using string::id;
using string::hex;
using string::num;
using string::name;
using string::user;
using string::value;
using string::required;
using string::optional;
using string::literal_t;

using string::token_t;
using string::parser_t;

typedef fmt::StringRef string_ref;

}  // namespace

namespace {

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

    auto operator()(const attribute::view_t::function_type& value) const -> void {
        value(writer);
    }
};

class pattern_visitor_t : public boost::static_visitor<> {
    writer_t& wr;
    const view_of<attribute_t>::type& attribute;

public:
    pattern_visitor_t(writer_t& wr, const view_of<attribute_t>::type& attribute) noexcept :
        wr(wr),
        attribute(attribute)
    {}

    auto operator()(const literal_t& value) -> void {
        wr.inner << value.value;
    }

    auto operator()(const ph::attribute<name>& value) -> void {
        // TODO: Surround with braces depending on a spec type.
        wr.write(value.format, string_ref(attribute.first.data(), attribute.first.size()));
    }

    auto operator()(const ph::attribute<value>& value) -> void {
        // TODO: Surround with braces depending on both spec and attribute types.
        boost::apply_visitor(view_visitor<spec>(wr, value.format), attribute.second.inner().value);
    }
};

class evaluator_t {
    const ph::leftover_t& placeholder;

public:
    explicit constexpr evaluator_t(const ph::leftover_t& placeholder) noexcept :
        placeholder(placeholder)
    {}

    auto eval(writer_t& wr, const attribute_pack& pack) -> void {
        bool first = true;
        for (const auto& attributes : pack) {
            for (const auto& attribute : attributes.get()) {
                if (first) {
                    first = false;
                    wr.inner << placeholder.prefix;
                } else {
                    wr.inner << placeholder.separator;
                }

                pattern_visitor_t visitor(wr, attribute);
                for (const auto& token : placeholder.tokens) {
                    boost::apply_visitor(visitor, token);
                }
            }
        }

        // TODO: Resize instead? Measure.
        if (!first) {
            wr.inner << placeholder.suffix;
        }
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
        if (token.gmtime) {
            ::gmtime_r(&time, &tm);
        } else {
            ::localtime_r(&time, &tm);
        }

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
        writer_t wr;
        evaluator_t(token)
            .eval(wr, record.attributes());

        // Seems like cppformat doesn't initializes the data pointer until required which leads to
        // conditional jump or move which depends on uninitialised value.
        if (wr.inner.size() > 0) {
            writer.write(token.spec, string_ref(wr.inner.data(), wr.inner.size()));
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

auto tokenize(const std::string& pattern) -> std::vector<token_t> {
    std::vector<token_t> tokens;

    parser_t parser(pattern);
    while (auto token = parser.next()) {
        tokens.emplace_back(token.get());
    }

    return tokens;
}

}  // namespace

// TODO: Decompose `throw std::invalid_argument("token not found");` case.

class string_t : public formatter_t {
    severity_map sevmap;
    std::vector<token_t> tokens;

public:
    explicit string_t(const std::string& pattern) :
        tokens(tokenize(pattern))
    {
        sevmap = [](int severity, const std::string& spec, writer_t& writer) {
            writer.write(spec, severity);
        };
    }

    string_t(const std::string& pattern, severity_map sevmap) :
        sevmap(std::move(sevmap)),
        tokens(tokenize(pattern))
    {}

    auto format(const record_t& record, writer_t& writer) -> void override {
        const visitor_t visitor(writer, record, sevmap);

        for (const auto& token : tokens) {
            boost::apply_visitor(visitor, token);
        }
    }
};

}  // namespace formatter

namespace experimental {

using formatter::severity_map;
using formatter::string_t;

class builder<string_t>::inner_t {
public:
    std::string pattern;
    severity_map sevmap;
};

builder<string_t>::builder(std::string pattern) :
    p(new inner_t{std::move(pattern), {}}, deleter_t())
{}

auto builder<string_t>::mapping(formatter::severity_map sevmap) && -> builder&& {
    p->sevmap = std::move(sevmap);
    return std::move(*this);
}

auto builder<string_t>::build() && -> std::unique_ptr<formatter_t> {
    if (p->sevmap) {
        return blackhole::make_unique<string_t>(std::move(p->pattern), std::move(p->sevmap));
    } else {
        return blackhole::make_unique<string_t>(std::move(p->pattern));
    }
}

auto factory<string_t>::type() const noexcept -> const char* {
    return "string";
}

auto factory<string_t>::from(const config::node_t& config) const -> std::unique_ptr<formatter_t> {
    auto pattern = config["pattern"].to_string().get();

    if (auto mapping = config["sevmap"]) {
        std::vector<std::string> sevmap;
        mapping.each([&](const config::node_t& config) {
            sevmap.emplace_back(config.to_string());
        });

        auto fn = [=](std::size_t severity, const std::string& spec, writer_t& writer) {
            if (severity < sevmap.size()) {
                writer.write(spec, sevmap[severity]);
            } else {
                writer.write(spec, severity);
            }
        };

        return blackhole::make_unique<string_t>(std::move(pattern), std::move(fn));
    }

    return blackhole::make_unique<string_t>(std::move(pattern));
}

}  // namespace experimental

template auto deleter_t::operator()(experimental::builder<formatter::string_t>::inner_t* value) -> void;

}  // namespace v1
}  // namespace blackhole
