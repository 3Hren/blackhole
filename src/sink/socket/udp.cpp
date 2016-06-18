#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/stdext/string_view.hpp"
#include "blackhole/sink/socket/udp.hpp"

#include "blackhole/detail/memory.hpp"
#include "blackhole/detail/util/optional.hpp"

#include "udp.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace socket {

udp_t::udp_t(const std::string& host, std::uint16_t port) :
    socket(io_service)
{
    boost::asio::ip::udp::resolver resolver(io_service);
    boost::asio::ip::udp::resolver::query query(host, boost::lexical_cast<std::string>(port),
        boost::asio::ip::udp::resolver::query::flags::numeric_service);
    endpoint_ = *resolver.resolve(query);

    socket.open(endpoint_.protocol());
}

auto udp_t::endpoint() const -> const endpoint_type& {
    return endpoint_;
}

auto udp_t::emit(const record_t&, const string_view& formatted) -> void {
    socket.send_to(boost::asio::buffer(formatted.data(), formatted.size()), endpoint_);
}

}  // namespace socket
}  // namespace sink

using sink::socket::udp_t;

using detail::util::value_or;

auto factory<udp_t>::type() const noexcept -> const char* {
    return "udp";
}

auto factory<udp_t>::from(const config::node_t& config) const -> std::unique_ptr<sink_t> {
    const auto host = value_or(config["host"].to_string(), []() -> std::string {
        throw std::invalid_argument(R"(parameter "host" is required)");
    });

    const auto port = value_or(config["port"].to_uint64(), []() -> std::uint64_t {
        throw std::invalid_argument(R"(parameter "port" is required)");
    });

    return blackhole::make_unique<udp_t>(host, static_cast<std::uint16_t>(port));
}

}  // namespace v1
}  // namespace blackhole
