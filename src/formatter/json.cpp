#include "blackhole/formatter/json.hpp"

#include <boost/variant/apply_visitor.hpp>

#define RAPIDJSON_HAS_STDSTRING 1
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

template<typename Allocator>
struct visitor_t {
    typedef void result_type;

    rapidjson::Value& node;
    Allocator& allocator;

    const record_t& record;

    const string_view& name;

    template<typename T>
    auto operator()(T value) -> void {
        BLACKHOLE_UNIMPLEMENTED();
    }

    auto operator()(std::int64_t value) -> void {
        node.AddMember(rapidjson::StringRef(name.data(), name.size()), value, allocator);
    }

    auto operator()(const string_view& value) -> void {
        node.AddMember(rapidjson::StringRef(name.data(), name.size()),
            rapidjson::StringRef(value.data(), value.size()), allocator);
    }
};

}  // namespace

class json_t::route_t {
public:
    rapidjson::Pointer pointer;

    route_t(const std::string& source) : pointer(source) {}
    route_t(rapidjson::Pointer pointer) : pointer(pointer) {}

    auto append(const std::string& name) const -> route_t {
        return {pointer.Append(name)};
    }
};

json_t::json_t() :
    base(new route_t(""))
{}

json_t::json_t(routing_t routing) :
    base(new route_t(routing.unspecified))
{
    for (const auto& route : routing.specified) {
        for (const auto& attribute : route.second) {
            this->routing.insert({attribute, route_t(route.first)});
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

    const auto& message = record.message();
    // root.AddMember("message", rapidjson::StringRef(message.data(), message.size()), root.GetAllocator());

    // for NAME and VALUE do apply.
    //  find ROUTE by NAME in ROUTING
    //   +: ROUTE.Set(NAME, VALUE).
    //       map NAME, then setter overloads(node).
    //   -: DEFAULT_ROUTE == nullptr?
    //    +: setter overloads(root).
    //    -: setter overloads(node)

    // routing.get(name) ->
    //  pointer if route is static.
    //  pointer if route is dynamic.
    //  pointer if route is default.

    /// fn apply(...)
    const auto it = routing.find("message");
    // TODO: Replace with find with default.
    if (it != routing.end()) {
        it->second.pointer.GetWithDefault(root, rapidjson::kObjectType)
            .AddMember("message", rapidjson::StringRef(message.data(), message.size()), root.GetAllocator());
    } else {
        base->pointer.GetWithDefault(root, rapidjson::kObjectType)
            .AddMember("message", rapidjson::StringRef(message.data(), message.size()), root.GetAllocator());
    }

    for (const auto& attributes : record.attributes()) {
        for (const auto& attribute : attributes.get()) {
            /// mapping ::= [attribute.name -> attribute.name].
            /// routing ::= [route -> [attribute.name]] if config,
            ///         ::= [attribute.name -> route] otherwise.
            ///                                variant ::= record -> attribute.

            // auto it = routing.find(attribute.name);
            // if (it != routing.end()) {
            //     auto (pointer, map) = *it;
            //     // Pointer value.
            //     if (auto node = pointer.Get()) {
            //         auto name = mapped(attribute.name);
            //         boost::apply_visitor(visitor_t{node}, attribute.value.inner().value);
            //     } else {}
            // }

            const auto it = routing.find(attribute.first.to_string());
            if (it != routing.end()) {
                auto& node = it->second.pointer.GetWithDefault(root, rapidjson::kObjectType);
                visitor_t<decltype(root.GetAllocator())> visitor{node, root.GetAllocator(), record, attribute.first};
                boost::apply_visitor(visitor, attribute.second.inner().value);
            } else {
                auto& node = base->pointer.GetWithDefault(root, rapidjson::kObjectType);
                visitor_t<decltype(root.GetAllocator())> visitor{node, root.GetAllocator(), record, attribute.first};
                boost::apply_visitor(visitor, attribute.second.inner().value);
            }
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
