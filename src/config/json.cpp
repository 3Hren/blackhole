#include "json.hpp"

#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include "blackhole/detail/memory.hpp"

namespace blackhole {
inline namespace v1 {
namespace config {

auto factory_traits<json_t>::construct(std::istream& stream) -> std::unique_ptr<factory_t> {
    return blackhole::make_unique<factory<json_t>>(stream);
}

auto factory_traits<json_t>::construct(std::istream&& stream) -> std::unique_ptr<factory_t> {
    return blackhole::make_unique<factory<json_t>>(std::move(stream));
}

factory<json_t>::factory(std::istream& stream) :
    node(doc)
{
    initialize(stream);
}

factory<json_t>::factory(std::istream&& stream) :
    node(doc)
{
    initialize(stream);
}

auto factory<json_t>::config() const noexcept -> const node_t& {
    return node;
}

auto factory<json_t>::initialize(std::istream& stream) -> void {
    rapidjson::IStreamWrapper wrapper(stream);
    doc.ParseStream<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag>(wrapper);

    if (doc.HasParseError()) {
        throw std::invalid_argument("parse error at offset " +
            boost::lexical_cast<std::string>(doc.GetErrorOffset()) +
            ": " + rapidjson::GetParseError_En(doc.GetParseError()));
    }
}

}  // namespace config
}  // namespace v1
}  // namespace blackhole
