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
}  // namespace blackhole
