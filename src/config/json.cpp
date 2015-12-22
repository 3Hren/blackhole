#include "blackhole/config/json.hpp"

#include <fstream>

#include <rapidjson/document.h>

#include "blackhole/detail/config/json.hpp"

namespace blackhole {
namespace config {

class factory<json_t>::inner_t {
public:
    rapidjson::Document doc;
    const config::json_t config;

    inner_t() : config(doc) {}
};

factory<json_t>::factory(const std::string& path) :
    inner(new inner_t)
{
    std::ifstream stream(path);
    std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    inner->doc.Parse<0>(content.c_str());

    if (inner->doc.HasParseError()) {
        throw std::invalid_argument("parse error");
    }
}

factory<json_t>::factory(factory&& other) noexcept = default;

factory<json_t>::~factory() = default;

auto factory<json_t>::config() const noexcept -> const config_t& {
    return inner->config;
}

}  // namespace config
}  // namespace blackhole
