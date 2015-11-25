#include "blackhole/sink/null.hpp"

#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/record.hpp"

namespace blackhole {
namespace sink {

auto
null_t::filter(const record_t&) -> bool {
    return false;
}

auto
null_t::execute(const record_t&, const string_view&) -> void {}

}  // namespace sink

auto
factory<sink::null_t>::type() -> const char* {
    return "null";
}

auto
factory<sink::null_t>::from(const config_t&) -> sink::null_t {
    return sink::null_t();
}

}  // namespace blackhole
