#include "blackhole/config/json.hpp"

#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include "blackhole/detail/config/json.hpp"

namespace blackhole {
inline namespace v1 {
namespace config {

class factory<json_t>::inner_t {
public:
    rapidjson::Document doc;
    const config::json_t node;

    inner_t() : node(doc) {}
};

factory<json_t>::factory(std::istream& stream) :
    inner(new inner_t)
{
    initialize(stream);
}

factory<json_t>::factory(std::istream&& stream) :
    inner(new inner_t)
{
    initialize(stream);
}

factory<json_t>::factory(factory&& other) noexcept = default;

factory<json_t>::~factory() = default;

auto factory<json_t>::operator=(factory&& other) noexcept -> factory& = default;

auto factory<json_t>::config() const noexcept -> const node_t& {
    return inner->node;
}

auto factory<json_t>::initialize(std::istream& stream) -> void {
    std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    inner->doc.Parse<0>(content.c_str());

    if (inner->doc.HasParseError()) {
        throw std::invalid_argument("parse error at offset " +
            boost::lexical_cast<std::string>(inner->doc.GetErrorOffset()) +
            ": " + rapidjson::GetParseError_En(inner->doc.GetParseError()));
    }
}

}  // namespace config
}  // namespace v1
}  // namespace blackhole
