#include "blackhole/sink/null.hpp"

#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/record.hpp"
#include "blackhole/sink.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

class null_t : public sink_t {
public:
    auto emit(const record_t&, const string_view&) -> void override {}
};

}  // namespace sink

auto factory<sink::null_t>::type() -> const char* {
    return "null";
}

auto factory<sink::null_t>::from(const config::node_t&) -> sink::null_t {
    return sink::null_t();
}

namespace experimental {

auto factory<sink::null_t>::type() const noexcept -> const char* {
    return "null";
}

auto factory<sink::null_t>::from(const config::node_t&) const -> std::unique_ptr<sink_t> {
    return std::unique_ptr<sink::null_t>(new sink::null_t);
}

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
