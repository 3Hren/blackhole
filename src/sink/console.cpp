#include "blackhole/sink/console.hpp"

#include <iostream>

#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {
namespace sink {

auto console_t::filter(const record_t&) -> bool {
    return true;
}

auto console_t::execute(const record_t&, const string_view& formatted) -> void {
    std::cout.write(formatted.data(), static_cast<std::streamsize>(formatted.size()));
    std::cout << std::endl;
}

}  // namespace sink

auto
factory<sink::console_t>::type() -> const char* {
    return "console";
}

auto
factory<sink::console_t>::from(const config_t&) -> sink::console_t {
    return {};
}

}  // namespace blackhole
