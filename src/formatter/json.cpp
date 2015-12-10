#include "blackhole/formatter/json.hpp"

#include <array>

#include <boost/variant/apply_visitor.hpp>

#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1
#endif
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/pointer.h>

#include "blackhole/record.hpp"
#include "blackhole/extensions/writer.hpp"

#include "blackhole/detail/attribute.hpp"
#include "blackhole/detail/unimplemented.hpp"

namespace blackhole {
namespace formatter {
namespace json {

class config_t::inner_t {
public:
    bool unique;
    bool newline;

    struct {
        std::map<std::string, std::vector<std::string>> specified;
        std::string unspecified;
    } routing;

    std::unordered_map<std::string, std::string> mapping;

    inner_t() :
        unique(false),
        newline(false)
    {}
};

config_t::config_t() :
    inner(new inner_t)
{}

config_t::config_t(config_t&& other) = default;

config_t::~config_t() = default;

auto config_t::operator=(config_t&& other) -> config_t& = default;

auto config_t::route(std::string rest) -> config_t& {
    inner->routing.unspecified = std::move(rest);
    return *this;
}

auto config_t::route(std::string route, std::vector<std::string> attributes) -> config_t& {
    inner->routing.specified[std::move(route)] = std::move(attributes);
    return *this;
}

auto config_t::rename(std::string from, std::string to) -> config_t& {
    inner->mapping[std::move(from)] = std::move(to);
    return *this;
}

auto config_t::config() const noexcept -> const inner_t& {
    return *inner;
}

}  // namespace json

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
};

/// A RapidJSON Stream concept implementation required to avoid intermediate buffer allocation.
struct stream_t {
    typedef char Ch;

    writer_t& wr;

    /// Writes a character directly into the underlying buffer.
    auto Put(Ch c) -> void {
        wr.inner << c;
    }

    /// Does nothing, because there is no intermediate buffer.
    auto Flush() -> void {}
};

}  // namespace

class json_t::factory_t {
public:
    // A JSON routing pointer for attributes that weren't mentioned in `routing` map.
    rapidjson::Pointer rest;
    // Routing map from attribute name to its JSON pointer.
    std::map<std::string, rapidjson::Pointer> routing;

    std::unordered_map<std::string, std::string> mapping;

    factory_t(config_type config) :
        rest(config.config().routing.unspecified),
        mapping(std::move(config.config().mapping))
    {
        for (const auto& route : config.config().routing.specified) {
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
class json_t::builder {
    Document& root;
    const record_t& record;
    factory_t& factory;

public:
    builder(Document& root, const record_t& record, factory_t& factory) :
        root(root),
        record(record),
        factory(factory)
    {}

    auto message() -> void {
        apply("message", record.message());
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
        const auto renamed = factory.renamed(name);
        visitor_t visitor{factory.get(name, root), root.GetAllocator(), renamed};
        visitor(value);
    }

    auto apply(const string_view& name, const attribute::view_t& value) -> void {
        const auto renamed = factory.renamed(name);
        visitor_t visitor{factory.get(name, root), root.GetAllocator(), renamed};
        boost::apply_visitor(visitor, value.inner().value);
    }
};

template<typename Document>
auto json_t::factory_t::create(Document& root, const record_t& record) ->
    builder<Document>
{
    return builder<Document>{root, record, *this};
}

json_t::json_t() :
    factory(new factory_t(json::config_t()))
{}

json_t::json_t(config_type config) :
    factory(new factory_t(std::move(config)))
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

    auto builder = factory->create(root, record);
    builder.message();
    builder.severity();
    builder.timestamp();
    builder.attributes();

    builder.build(writer);

    // TODO: Add newline if required. Obtained through config.
    // if (config.newline) {
    //     writer << '\n';
    // }
}

}  // namespace formatter
}  // namespace blackhole
