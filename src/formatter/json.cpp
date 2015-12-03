#include "blackhole/formatter/json.hpp"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "blackhole/record.hpp"
#include "blackhole/extensions/writer.hpp"

namespace blackhole {
namespace formatter {

auto json_t::format(const record_t& record, writer_t& writer) -> void {
    rapidjson::Document root;
    root.SetObject();

    // TODO: Make flattened range
    // TODO: Make uniqued range.

    const auto& message = record.message();
    root.AddMember("message", rapidjson::StringRef(message.data(), message.size()), root.GetAllocator());

    // const visitor_t visitor(&root, record, sevmap);

    // TODO: Update my knowledge how to properly to this. Anything might change.
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
