#include "blackhole/formatter/json.hpp"

#include <boost/variant/apply_visitor.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "blackhole/record.hpp"
#include "blackhole/extensions/writer.hpp"

#include "blackhole/detail/attribute.hpp"
#include "blackhole/detail/unimplemented.hpp"

namespace blackhole {
namespace formatter {

namespace {

struct visitor_t {
    typedef void result_type;

    rapidjson::Document& root;
    const record_t& record;

    const string_view& name;

    template<typename T>
    auto operator()(T value) -> void {
        BLACKHOLE_UNIMPLEMENTED();
    }

    auto operator()(std::int64_t value) -> void {
        root.AddMember(rapidjson::StringRef(name.data(), name.size()), value, root.GetAllocator());
    }

    auto operator()(const string_view& value) -> void {
        root.AddMember(rapidjson::StringRef(name.data(), name.size()),
            rapidjson::StringRef(value.data(), value.size()), root.GetAllocator());
    }
};

}  // namespace

auto json_t::format(const record_t& record, writer_t& writer) -> void {
    rapidjson::Document root;
    root.SetObject();

    // TODO: Make flattened range
    // TODO: Make uniqued range.
    // TODO: Try to use Pointer API. Measure.
    // TODO: Try to use `AutoUTF<>` or `AutoUTFOutputStream` for UTF-8 validation.

    const auto& message = record.message();
    root.AddMember("message", rapidjson::StringRef(message.data(), message.size()), root.GetAllocator());

    for (const auto& attributes : record.attributes()) {
        for (const auto& attribute : attributes.get()) {
            visitor_t visitor{root, record, attribute.first};
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
