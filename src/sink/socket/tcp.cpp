#include "blackhole/sink/socket/tcp.hpp"

#include "blackhole/detail/sink/socket/tcp.hpp"
#include "blackhole/detail/util/optional.hpp"

namespace blackhole {
inline namespace v1 {

using detail::util::value_or;

auto factory<sink::socket::tcp_t>::type() -> const char* {
    return "tcp";
}

auto factory<sink::socket::tcp_t>::from(const config::node_t& config) -> sink::socket::tcp_t {
    const auto host = value_or(config["host"].to_string(), []() -> std::string {
        throw std::invalid_argument(R"(parameter "host" is required)");
    });

    const auto port = value_or(config["port"].to_uint64(), []() -> std::uint64_t {
        throw std::invalid_argument(R"(parameter "port" is required)");
    });

    return {host, static_cast<std::uint16_t>(port)};
}

}  // namespace v1
}  // namespace blackhole
