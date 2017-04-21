#include "blackhole/formatter/json.hpp"

#include <array>
#include <set>
#include <unordered_map>

#include <boost/optional/optional.hpp>
#include <boost/variant/apply_visitor.hpp>

#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1
#endif
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/pointer.h>

#include "blackhole/attribute.hpp"
#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/record.hpp"
#include "blackhole/extensions/writer.hpp"

#include "blackhole/detail/memory.hpp"
#include "blackhole/detail/util/deleter.hpp"

#include "../attribute.hpp"
#include "../datetime.hpp"
#include "json.hpp"

namespace blackhole {
inline namespace v1 {
namespace formatter {
namespace {

struct visitor_t {
    typedef void result_type;

    rapidjson::Value& node;
    rapidjson::MemoryPoolAllocator<>& allocator;

    const string_view& name;

    auto operator()(std::nullptr_t) -> void {
        node.AddMember(rapidjson::StringRef(name.data(), name.size()), rapidjson::kNullType, allocator);
    }

    // For `bool`, `std::int64_t`, `std::uint64_t` and `double` types.
    template<typename T>
    auto operator()(T value) -> void {
        static_assert(
            std::is_same<T, bool>::value ||
            std::is_same<T, std::int64_t>::value ||
            std::is_same<T, std::uint64_t>::value ||
            std::is_same<T, double>::value, "type mismatch");
        node.AddMember(rapidjson::StringRef(name.data(), name.size()), value, allocator);
    }

    auto operator()(const string_view& value) -> void {
        node.AddMember(rapidjson::StringRef(name.data(), name.size()),
            rapidjson::StringRef(value.data(), value.size()), allocator);
    }

    // For temporary buffers, rapidjson will copy the data.
    auto operator()(const char* data, std::size_t size) -> void {
        node.AddMember(rapidjson::StringRef(name.data(), name.size()),
            rapidjson::Value(data, static_cast<unsigned int>(size), allocator), allocator);
    }

    auto operator()(const attribute::view_t::function_type& value) -> void {
        writer_t wr;
        value(wr);

        node.AddMember(rapidjson::StringRef(name.data(), name.size()),
            wr.result().to_string(), allocator);
    }
};

// TODO: Decompose. Almost the same class is in `string.cpp`.
class view_visitor : public boost::static_visitor<> {
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

/// A RapidJSON Stream concept implementation required to avoid intermediate buffer allocation.
struct stream_t {
    typedef char Ch;

    writer_t& wr;

    /// Writes a character directly into the underlying buffer.
    // TODO: Seems like writing string one-by-one affects the performance. See similar benchmarks
    // that differs only with input string length.
    auto Put(Ch c) -> void {
        wr.inner << c;
    }

    /// Does nothing, because there is no intermediate buffer.
    auto Flush() -> void {}
};

}  // namespace

class json_t::properties_t {
public:
    bool unique;
    bool newline;

    std::function<void(const record_t::time_point& time, writer_t& wr)> timestamp;
    std::vector<std::string> severity;

    struct {
        std::map<std::string, std::vector<std::string>> specified;
        std::string unspecified;
    } routing;

    std::unordered_map<std::string, std::string> mapping;
    std::unordered_map<std::string, std::string> formatting;

    properties_t() :
        unique(false),
        newline(false)
    {}
};

class json_t::inner_t {
    template<typename> class builder;

public:
    // A JSON routing pointer for attributes that weren't mentioned in `routing` map.
    rapidjson::Pointer rest;
    // Routing map from attribute name to its JSON pointer.
    std::map<std::string, rapidjson::Pointer> routing;

    std::unordered_map<std::string, std::string> mapping;
    std::unordered_map<std::string, std::string> formatting;

    bool unique;
    bool newline;

    std::function<void(const record_t::time_point& time, writer_t& wr)> timestamp;
    std::vector<std::string> severity;

    inner_t(json_t::properties_t properties) :
        rest(properties.routing.unspecified),
        mapping(std::move(properties.mapping)),
        formatting(std::move(properties.formatting)),
        unique(properties.unique),
        newline(properties.newline),
        timestamp(properties.timestamp),
        severity(std::move(properties.severity))
    {
        for (const auto& route : properties.routing.specified) {
            for (const auto& name : route.second) {
                routing.insert({name, rapidjson::Pointer(route.first)});
            }
        }
    }

    template<typename Document>
    auto get(const string_view& name, Document& root) -> rapidjson::Value& {
        // TODO: Here we can avoid a temporary string construction by using multi indexed
        // containers.
        const auto it = routing.find(name.to_string());

        if (it == routing.end()) {
            return rest.GetWithDefault(root, rapidjson::kObjectType);
        } else {
            return it->second.GetWithDefault(root, rapidjson::kObjectType);
        }
    }

    template<typename Document>
    auto create(Document& root, const record_t& record) -> builder<Document>;

    auto renamed(const string_view& name) const -> string_view {
        const auto it = mapping.find(name.to_string());

        if (it == mapping.end()) {
            return name;
        } else {
            return it->second;
        }
    }
};

template<typename Document>
class json_t::inner_t::builder {
    Document& root;
    const record_t& record;
    inner_t& inner;

public:
    builder(Document& root, const record_t& record, inner_t& inner) :
        root(root),
        record(record),
        inner(inner)
    {}

    auto message() -> void {
        apply("message", record.formatted());
    }

    auto process() -> void {
        apply("process", record.pid());
    }

    auto lwp() -> void {
        #ifdef __linux__
                apply("lwp", record.lwp());
        #endif
    }

    auto thread() -> void {
        writer_t wr;
        // TODO: Encapsulate.
#ifdef __linux__
            wr.write("{:#x}", record.tid());
#elif __APPLE__
            wr.write("{:#x}", reinterpret_cast<unsigned long>(record.tid()));
#endif
        apply("thread", wr.inner.data(), wr.inner.size());
    }

    auto severity() -> void {
        const auto id = static_cast<std::size_t>(record.severity());

        if (inner.severity.empty() || id >= inner.severity.size()) {
            apply("severity", static_cast<std::int64_t>(record.severity()));
        } else {
            apply("severity", string_view(inner.severity[id].data(), inner.severity[id].size()));
        }
    }

    auto timestamp() -> void {
        if (inner.timestamp) {
            writer_t wr;
            inner.timestamp(record.timestamp(), wr);
            apply("timestamp", wr.inner.data(), wr.inner.size());
        } else {
            apply("timestamp", std::chrono::duration_cast<
                std::chrono::microseconds
            >(record.timestamp().time_since_epoch()).count());
        }
    }

    auto build(writer_t& writer) -> void {
        stream_t stream{writer};
        rapidjson::Writer<stream_t> wr(stream);
        root.Accept(wr);
    }

    auto attributes() -> void {
        if (inner.unique) {
            // TODO: Small buffer optimization is possible here (see stack allocator with arena).
            std::set<string_view> set;

            for (const auto& attributes : record.attributes()) {
                for (const auto& attribute : attributes.get()) {
                    if (set.insert(attribute.first).second) {
                        apply(attribute.first, attribute.second);
                    }
                }
            }
            return;
        }

        // TODO: Make flattened range
        // TODO: Make uniqued range.
        for (const auto& attributes : record.attributes()) {
            for (const auto& attribute : attributes.get()) {
                apply(attribute.first, attribute.second);
            }
        }
    }

private:
    template<typename T>
    auto apply(const string_view& name, const T& value) -> void {
        const auto renamed = inner.renamed(name);
        visitor_t visitor{inner.get(name, root), root.GetAllocator(), renamed};

        const auto it = inner.formatting.find(std::string(name.data(), name.size()));
        if (it != std::end(inner.formatting)) {
            writer_t wr;
            wr.write(it->second, value);
            visitor(wr.inner.data(), wr.inner.size());
        } else {
            visitor(value);
        }
    }

    auto apply(const string_view& name, const char* data, std::size_t size) -> void {
        const auto renamed = inner.renamed(name);
        visitor_t visitor{inner.get(name, root), root.GetAllocator(), renamed};

        const auto it = inner.formatting.find(std::string(name.data(), name.size()));
        if (it != std::end(inner.formatting)) {
            writer_t wr;
            wr.write(it->second, fmt::StringRef(data, size));
            visitor(wr.inner.data(), wr.inner.size());
        } else {
            visitor(data, size);
        }
    }

    auto apply(const string_view& name, const attribute::view_t& value) -> void {
        const auto renamed = inner.renamed(name);
        visitor_t visitor{inner.get(name, root), root.GetAllocator(), renamed};

        const auto it = inner.formatting.find(std::string(name.data(), name.size()));
        if (it != std::end(inner.formatting)) {
            writer_t wr;
            boost::apply_visitor(view_visitor(wr, it->second), value.inner().value);
            visitor(wr.inner.data(), wr.inner.size());
        } else {
            boost::apply_visitor(visitor, value.inner().value);
        }
    }
};

template<typename Document>
auto json_t::inner_t::create(Document& root, const record_t& record) ->
    builder<Document>
{
    return builder<Document>{root, record, *this};
}

json_t::json_t() :
    inner(new inner_t(properties_t()))
{}

json_t::json_t(properties_t properties) :
    inner(new inner_t(std::move(properties)))
{}

auto json_t::newline() const noexcept -> bool {
    return inner->newline;
}

auto json_t::unique() const noexcept -> bool {
    return inner->unique;
}

auto json_t::format(const record_t& record, writer_t& writer) -> void {
    typedef rapidjson::GenericDocument<
        rapidjson::UTF8<>,
        rapidjson::MemoryPoolAllocator<>,
        rapidjson::MemoryPoolAllocator<>
    > document_type;

    std::array<char, 4096> value_buffer;
    std::array<char, 1024> parse_buffer;
    rapidjson::MemoryPoolAllocator<> value_allocator(value_buffer.data(), value_buffer.size());
    rapidjson::MemoryPoolAllocator<> parse_allocator(parse_buffer.data(), parse_buffer.size());

    document_type root(&value_allocator, parse_buffer.size(), &parse_allocator);
    root.SetObject();

    // TODO: Try to use `AutoUTF<>` or `AutoUTFOutputStream` for UTF-8 validation.

    auto builder = inner->create(root, record);
    builder.message();
    builder.lwp();
    builder.thread();
    builder.process();
    builder.severity();
    builder.timestamp();
    builder.attributes();

    builder.build(writer);

    if (inner->newline) {
        writer.inner << '\n';
    }
}

}  // namespace formatter

using formatter::json_t;

class builder<json_t>::inner_t : public json_t::properties_t {};

builder<json_t>::builder() :
    d(new inner_t{}, deleter_t())
{}

auto builder<json_t>::route(std::string route) & -> builder& {
    d->routing.unspecified = std::move(route);
    return *this;
}

auto builder<json_t>::route(std::string route) && -> builder&& {
    d->routing.unspecified = std::move(route);
    return std::move(*this);
}

auto builder<json_t>::route(std::string route, std::vector<std::string> attributes) & -> builder& {
    d->routing.specified[std::move(route)] = std::move(attributes);
    return *this;
}

auto builder<json_t>::route(std::string route, std::vector<std::string> attributes) && -> builder&& {
    d->routing.specified[std::move(route)] = std::move(attributes);
    return std::move(*this);
}

auto builder<json_t>::rename(std::string from, std::string to) & -> builder& {
    d->mapping[std::move(from)] = std::move(to);
    return *this;
}

auto builder<json_t>::rename(std::string from, std::string to) && -> builder&& {
    d->mapping[std::move(from)] = std::move(to);
    return std::move(*this);
}

auto builder<json_t>::unique() & -> builder& {
    d->unique = true;
    return *this;
}

auto builder<json_t>::unique() && -> builder&& {
    return std::move(unique());
}

auto builder<json_t>::newline() & -> builder& {
    d->newline = true;
    return *this;
}

auto builder<json_t>::newline() && -> builder&& {
    d->newline = true;
    return std::move(*this);
}

auto builder<json_t>::severity(std::vector<std::string> sevmap) & -> builder& {
    d->severity = std::move(sevmap);
    return *this;
}

auto builder<json_t>::severity(std::vector<std::string> sevmap) && -> builder&& {
    d->severity = std::move(sevmap);
    return std::move(*this);
}

auto builder<json_t>::timestamp(const std::string& pattern) & -> builder& {
    auto generator = datetime::make_generator(pattern);

    d->timestamp = [=](const record_t::time_point& timestamp, writer_t& wr) {
        const auto time = record_t::clock_type::to_time_t(timestamp);
        const auto usec = std::chrono::duration_cast<
            std::chrono::microseconds
        >(timestamp.time_since_epoch()).count() % 1000000;

        std::tm tm;
        ::gmtime_r(&time, &tm);

        generator(wr.inner, tm, static_cast<std::uint64_t>(usec));
    };

    return *this;
}

auto builder<json_t>::timestamp(const std::string& pattern) && -> builder&& {
    return std::move(timestamp(pattern));
}

auto builder<json_t>::format(std::string name, std::string spec) & -> builder& {
    d->formatting[name] = spec;
    return *this;
}

auto builder<json_t>::format(std::string name, std::string spec) && -> builder&& {
    return std::move(format(std::move(name), std::move(spec)));
}

auto builder<json_t>::build() const && -> std::unique_ptr<formatter_t> {
    return blackhole::make_unique<json_t>(std::move(*d));
}

auto factory<json_t>::type() const noexcept -> const char* {
    return "json";
}

auto factory<json_t>::from(const config::node_t& config) const ->
    std::unique_ptr<formatter_t>
{
    builder<json_t> builder;

    if (auto unique = config["unique"].to_bool()) {
        if (unique.get()) {
            builder.unique();
        }
    }

    if (auto newline = config["newline"].to_bool()) {
        if (newline.get()) {
            builder.newline();
        }
    }

    if (auto mapping = config["mapping"]) {
        mapping.each_map([&](const std::string& key, const config::node_t& value) {
            builder.rename(key, value.to_string());
        });
    }

    if (auto formatting = config["formatting"]) {
        formatting.each_map([&](const std::string& key, const config::node_t& value) {
            builder.format(key, value.to_string());
        });
    }

    if (auto routing = config["routing"]) {
        routing.each_map([&](const std::string& key, const config::node_t& value) {
            try {
                // TODO: Probably it's right thing to explicitly check whether the value is string.
                value.to_string();
                builder.route(key);
                return;
            } catch (const std::logic_error&) {
                // Eat.
            }

            std::vector<std::string> attributes;
            value.each([&](const config::node_t& config) {
                attributes.emplace_back(config.to_string());
            });
            builder.route(key, std::move(attributes));
        });
    }

    if (auto mutate = config["mutate"]) {
        mutate.each_map([&](const std::string& key, const config::node_t& config) {
            if (key == "timestamp") {
                builder.timestamp(config.to_string());
            } else if (key == "severity") {
                std::vector<std::string> mapping;
                config.each([&](const config::node_t& severity) {
                    mapping.emplace_back(severity.to_string());
                });
                builder.severity(std::move(mapping));
            } else {
                throw std::invalid_argument("only \"timestamp\" and \"severity\" mutations are allowed now");
            }
        });
    }

    return std::move(builder).build();
}

template auto deleter_t::operator()(builder<formatter::json_t>::inner_t*) -> void;
template auto deleter_t::operator()(formatter::json_t::inner_t*) -> void;

}  // namespace v1
}  // namespace blackhole
