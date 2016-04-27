#include "blackhole/sink/null.hpp"

#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/record.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

class null_t : public sink_t {
public:
   auto emit(const record_t&, const string_view&) -> void {}
};

}  // namespace sink

auto factory<sink::null_t>::type() const noexcept -> const char* {
    return "null";
}

auto factory<sink::null_t>::from(const config::node_t&) const -> std::unique_ptr<sink_t> {
    return std::unique_ptr<sink_t>(new sink::null_t);
}

}  // namespace v1
}  // namespace blackhole
