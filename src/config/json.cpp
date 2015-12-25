#include "blackhole/config/json.hpp"

#include <fstream>

#include <rapidjson/document.h>

#include "blackhole/detail/config/json.hpp"

namespace blackhole {
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
    std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    inner->doc.Parse<0>(content.c_str());

    if (inner->doc.HasParseError()) {
        // TODO: More verbose error message (i.e line:column where error occurred).
        throw std::invalid_argument("parse error");
    }
}

factory<json_t>::factory(std::istream&& stream) :
    inner(new inner_t)
{
    std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    inner->doc.Parse<0>(content.c_str());

    if (inner->doc.HasParseError()) {
        // TODO: More verbose error message (i.e line:column where error occurred).
        throw std::invalid_argument("parse error");
    }
}

factory<json_t>::factory(factory&& other) noexcept = default;

factory<json_t>::~factory() = default;

auto factory<json_t>::operator=(factory&& other) noexcept -> factory& = default;

auto factory<json_t>::config() const noexcept -> const node_t& {
    return inner->node;
}

}  // namespace config
}  // namespace blackhole
