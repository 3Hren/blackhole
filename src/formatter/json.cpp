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
        BLACKHOLE_UNIMPLEMENTED();
    }

    auto operator()(bool) -> void {
        BLACKHOLE_UNIMPLEMENTED();
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

}  // namespace

class json_t::inner_t {
public:
    rapidjson::Pointer rest;
    std::unordered_map<std::string, rapidjson::Pointer> routing;

    inner_t(std::string rest = std::string()) :
        rest(std::move(rest))
    {}

    auto get(const string_view& name, rapidjson::Document& root) -> rapidjson::Value& {
        const auto& route = find(routing, name.to_string(), [&]() -> const rapidjson::Pointer& {
            return rest;
        });

        return route.GetWithDefault(root, rapidjson::kObjectType);
    }

    auto apply(const string_view& name, const string_view& value, rapidjson::Document& root) -> void {
        visitor_t visitor{get(name, root), root.GetAllocator(), name};
        visitor(value);
    }

    auto apply(const string_view& name, int value, rapidjson::Document& root) -> void {
        apply(name, static_cast<std::int64_t>(value), root);
    }

    auto apply(const string_view& name, std::int64_t value, rapidjson::Document& root) -> void {
        visitor_t visitor{get(name, root), root.GetAllocator(), name};
        visitor(value);
    }

    auto apply(const string_view& name, const attribute::view_t& value, rapidjson::Document& root) -> void {
        visitor_t visitor{get(name, root), root.GetAllocator(), name};
        boost::apply_visitor(visitor, value.inner().value);
    }
};

json_t::json_t() :
    inner(new inner_t)
{}

json_t::json_t(routing_t routing) :
    inner(new inner_t(routing.unspecified))
{
    for (const auto& route : routing.specified) {
        for (const auto& attribute : route.second) {
            inner->routing.insert({attribute, rapidjson::Pointer(route.first)});
        }
    }
}

json_t::~json_t() {}

auto json_t::format(const record_t& record, writer_t& writer) -> void {
    rapidjson::Document root;
    root.SetObject();

    // TODO: Make flattened range
    // TODO: Make uniqued range.
    // TODO: Try to use `AutoUTF<>` or `AutoUTFOutputStream` for UTF-8 validation.

    inner->apply("message", record.message(), root);
    inner->apply("severity", record.severity(), root);

    const auto timestamp = std::chrono::duration_cast<
        std::chrono::microseconds
    >(record.timestamp().time_since_epoch()).count();
    inner->apply("timestamp", timestamp, root);

    for (const auto& attributes : record.attributes()) {
        for (const auto& attribute : attributes.get()) {
            inner->apply(attribute.first, attribute.second, root);
        }
    }

    // TODO: Update my knowledge how to properly do this. Anything might change.
    rapidjson::GenericStringBuffer<rapidjson::UTF8<>> buffer;
    // TODO: Leave this implementation for now. Measure. Then replace with cppformat writer.
    rapidjson::Writer<rapidjson::GenericStringBuffer<rapidjson::UTF8<>>> resultwriter(buffer);

    root.Accept(resultwriter);

    writer.inner << fmt::StringRef(buffer.GetString(), buffer.GetSize());

    // TODO: Add newline if required. Obtained through config.
    // if (config.newline) {
    //     writer << '\n';
    // }
}

}  // namespace formatter
}  // namespace blackhole
