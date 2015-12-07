#include "blackhole/formatter/json.hpp"

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

namespace {

struct visitor_t {
    typedef void result_type;

    rapidjson::Value& node;
    rapidjson::MemoryPoolAllocator<>& allocator;

    const string_view& name;

    auto operator()(std::nullptr_t) -> void {
        node.AddMember(rapidjson::StringRef(name.data(), name.size()), rapidjson::kNullType, allocator);
    }

    auto operator()(bool value) -> void {
        node.AddMember(rapidjson::StringRef(name.data(), name.size()), value, allocator);
    }

    auto operator()(std::int64_t value) -> void {
        node.AddMember(rapidjson::StringRef(name.data(), name.size()), value, allocator);
    }

    auto operator()(std::uint64_t value) -> void {
        node.AddMember(rapidjson::StringRef(name.data(), name.size()), value, allocator);
    }

    auto operator()(double value) -> void {
        BLACKHOLE_UNIMPLEMENTED();
    }

    auto operator()(const string_view& value) -> void {
        node.AddMember(rapidjson::StringRef(name.data(), name.size()),
            rapidjson::StringRef(value.data(), value.size()), allocator);
    }
};

template<typename Map, typename F>
inline auto find(const Map& map, const typename Map::key_type& key, const F& fn) ->
    typename Map::mapped_type
{
    const auto it = map.find(key);
    if (it == map.end()) {
        return fn();
    } else {
        return it->second;
    }
}

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
    std::unordered_map<std::string, rapidjson::Pointer> routing;

    factory_t() :
        rest("")
    {}

    factory_t(routing_t routing) :
        rest(routing.unspecified)
    {
        for (const auto& route : routing.specified) {
            for (const auto& name : route.second) {
                this->routing.insert({name, rapidjson::Pointer(route.first)});
            }
        }
    }

    auto get(const string_view& name, rapidjson::Document& root) -> rapidjson::Value& {
        const auto& route = find(routing, name.to_string(), [&]() -> const rapidjson::Pointer& {
            return rest;
        });

        return route.GetWithDefault(root, rapidjson::kObjectType);
    }

    auto create(rapidjson::Document& root, const record_t& record) -> builder_t;
};

class json_t::builder_t {
    factory_t& factory;
    const record_t& record;
    rapidjson::Document& root;

public:
    builder_t(factory_t& factory, const record_t& record, rapidjson::Document& root) :
        factory(factory),
        record(record),
        root(root)
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
        visitor_t visitor{factory.get(name, root), root.GetAllocator(), name};
        visitor(value);
    }

    auto apply(const string_view& name, const attribute::view_t& value) -> void {
        visitor_t visitor{factory.get(name, root), root.GetAllocator(), name};
        boost::apply_visitor(visitor, value.inner().value);
    }
};

auto json_t::factory_t::create(rapidjson::Document& root, const record_t& record) -> builder_t {
    return builder_t{*this, record, root};
}

json_t::json_t() :
    factory(new factory_t)
{}

json_t::json_t(routing_t routing) :
    factory(new factory_t(std::move(routing)))
{}

json_t::~json_t() {}

auto json_t::format(const record_t& record, writer_t& writer) -> void {
    rapidjson::Document root;
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
