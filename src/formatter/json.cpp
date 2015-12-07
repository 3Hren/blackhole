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

}  // namespace

class json_t::inner_t {
public:
    route_t rest;
    std::unordered_map<std::string, route_t> routing;

    inner_t(std::string rest) : rest(std::move(rest)) {}

    auto get(const string_view& name, rapidjson::Document& root) -> rapidjson::Value& {
        const auto it = routing.find(name.to_string());
        if (it == routing.end()) {
            return rest.pointer.GetWithDefault(root, rapidjson::kObjectType);
        } else {
            return it->second.pointer.GetWithDefault(root, rapidjson::kObjectType);
        }
    }
};

json_t::json_t() :
    inner(new inner_t(""))
{}

json_t::json_t(routing_t routing) :
    inner(new inner_t(routing.unspecified))
{
    for (const auto& route : routing.specified) {
        for (const auto& attribute : route.second) {
            inner->routing.insert({attribute, route_t(route.first)});
        }
    }
}

json_t::~json_t() {}

auto json_t::format(const record_t& record, writer_t& writer) -> void {
    rapidjson::Document root;
    root.SetObject();

    // TODO: Make flattened range
    // TODO: Make uniqued range.
    // TODO: Try to use Pointer API. Measure.
    // TODO: Try to use `AutoUTF<>` or `AutoUTFOutputStream` for UTF-8 validation.

    // apply("message", record.message(), allocator) -> void =>
    //   auto& node = routing.get("message", root);
    //     const auto it = map.find("message");
    //     if (it == map.end())
    //       return rest.pointer.GetWithDefault(root, rapidjson::kObjectType);
    //     else
    //       return it->second.pointer.GetWithDefault(root, rapidjson::kObjectType);
    //   visitor_t visitor(node, mapped("message"), allocator);
    //   visitor(record.message());
    //     node.AddMember("message", rapidjson::StringRef(message.data(), message.size()), allocator);
    // apply("severity", record.severity(), allocator);
    // ...
    // for (const auto& attribute : record.attributes().flattened()) {
    //   apply(attribute.name, attribute.value, allocator);
    // }

    {
        auto& node = inner->get("message", root);
        visitor_t visitor{node, root.GetAllocator(), "message"};
        visitor(record.message());
    }

    {
        auto& node = inner->get("severity", root);
        visitor_t visitor{node, root.GetAllocator(), "severity"};
        visitor(static_cast<std::int64_t>(record.severity()));
    }

    {
        auto& node = inner->get("timestamp", root);
        visitor_t visitor{node, root.GetAllocator(), "timestamp"};
        visitor(std::chrono::duration_cast<std::chrono::microseconds>(record.timestamp().time_since_epoch()).count());
    }

    for (const auto& attributes : record.attributes()) {
        for (const auto& attribute : attributes.get()) {
            auto& node = inner->get(attribute.first, root);
            visitor_t visitor{node, root.GetAllocator(), attribute.first};
            boost::apply_visitor(visitor, attribute.second.inner().value);
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
