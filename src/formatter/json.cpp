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

#include "blackhole/detail/attribute.hpp"

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

    auto operator()(const attribute::view_t::function_type& value) -> void {
        writer_t wr;
        value(wr);

        node.AddMember(rapidjson::StringRef(name.data(), name.size()),
            wr.result().to_string(), allocator);
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

    struct {
        std::map<std::string, std::vector<std::string>> specified;
        std::string unspecified;
    } routing;

    std::unordered_map<std::string, std::string> mapping;

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

    bool unique;
    bool newline;

    inner_t(json_t::properties_t properties) :
        rest(properties.routing.unspecified),
        mapping(std::move(properties.mapping)),
        unique(properties.unique),
        newline(properties.newline)
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

    auto severity() -> void {
        apply("severity", static_cast<std::int64_t>(record.severity()));
    }

    auto timestamp() -> void {
        apply("timestamp", std::chrono::duration_cast<
            std::chrono::microseconds
        >(record.timestamp().time_since_epoch()).count());
    }

    auto build(writer_t& writer) -> void {
        stream_t stream{writer};
        rapidjson::Writer<stream_t> wr(stream);
        root.Accept(wr);
    }

    auto attributes() -> void {
        if (inner.unique) {
            // TODO: Small buffer optimization is possible here (see stack allocator with arena).
            // TODO: Also consider using `unordered_set` instead. But it requires either manually or
            // indirectly implementing murmur3 hashing to be fully compatible with the Standard.
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
        visitor(value);
    }

    auto apply(const string_view& name, const attribute::view_t& value) -> void {
        const auto renamed = inner.renamed(name);
        visitor_t visitor{inner.get(name, root), root.GetAllocator(), renamed};
        boost::apply_visitor(visitor, value.inner().value);
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

json_t::~json_t() {}

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
    builder.severity();
    builder.timestamp();
    builder.attributes();

    builder.build(writer);

    if (inner->newline) {
        writer.inner << '\n';
    }
}

json_t::builder_t::builder_t() :
    properties(new properties_t)
{}

json_t::builder_t::~builder_t() = default;

auto json_t::builder_t::route(std::string route) -> builder_t& {
    properties->routing.unspecified = std::move(route);
    return *this;
}

auto json_t::builder_t::route(std::string route, std::vector<std::string> attributes) -> builder_t& {
    properties->routing.specified[std::move(route)] = std::move(attributes);
    return *this;
}

auto json_t::builder_t::rename(std::string from, std::string to) -> builder_t& {
    properties->mapping[std::move(from)] = std::move(to);
    return *this;
}

auto json_t::builder_t::unique() -> builder_t& {
    properties->unique = true;
    return *this;
}

auto json_t::builder_t::newline() -> builder_t& {
    properties->newline = true;
    return *this;
}

auto json_t::builder_t::build() const -> json_t {
    return {std::move(*properties)};
}

}  // namespace formatter

auto factory<formatter::json_t>::type() noexcept -> const char* {
    return "json";
}

auto factory<formatter::json_t>::from(const config::node_t& config) -> formatter::json_t {
    formatter::json_t::builder_t builder;

    if (auto unique = config["unique"].to_bool()) {
        if (unique.get()) {
            builder.unique();
        }
    }

    if (auto mapping = config["mapping"]) {
        mapping.each_map([&](const std::string& key, const config::node_t& value) {
            builder.rename(key, value.to_string());
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

    return builder.build();
}

}  // namespace v1
}  // namespace blackhole
