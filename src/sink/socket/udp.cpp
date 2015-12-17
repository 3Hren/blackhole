#include "blackhole/sink/socket/udp.hpp"

namespace blackhole {
namespace sink {
namespace socket {

namespace udp {

class inner_t {
public:
};

}  // namespace udp

udp_t::udp_t(const std::string& host, std::uint16_t port) {}

udp_t::~udp_t() = default;

auto udp_t::filter(const record_t&) -> bool {
    return true;
}

auto udp_t::execute(const record_t& record, const string_view& formatted) -> void {}

}  // namespace socket
}  // namespace sink

auto factory<sink::socket::udp_t>::type() -> const char* {
    return "udp";
}

}  // namespace blackhole
