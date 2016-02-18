#include <mutex>

#include <boost/asio/connect.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/extensions/format.hpp"
#include "blackhole/sink.hpp"
#include "blackhole/sink/socket/tcp.hpp"

#include "blackhole/detail/util/optional.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace socket {

typedef boost::asio::ip::tcp protocol_type;
typedef protocol_type::socket socket_type;
typedef protocol_type::endpoint endpoint_type;

struct tcp_t::data_t {
    std::string host;
    std::uint16_t port;

    boost::asio::io_service io_service;
    std::unique_ptr<socket_type> socket;

    mutable std::mutex mutex;
};

namespace {

/// Resolves specified host and tries to connect to the socket.
template<typename Protocol>
auto connect(boost::asio::io_service& ev, typename Protocol::socket& socket,
    const std::string& host, std::uint16_t port) -> void
{
    typename Protocol::resolver::iterator endpoint;

    try {
        typename Protocol::resolver resolver(ev);
        typename Protocol::resolver::query query(host, boost::lexical_cast<std::string>(port),
            Protocol::resolver::query::flags::numeric_service);
        endpoint = resolver.resolve(query);
    } catch (const boost::system::system_error& err) {
        throw std::system_error(err.code().value(), std::system_category(),
            fmt::format("failed to resolve {}:{} - {}", host, port, err.what()));
    }

    try {
        boost::asio::connect(socket, endpoint);
    } catch (const boost::system::system_error& err) {
        throw std::system_error(err.code().value(), std::system_category(),
            fmt::format("failed to connect to {}:{} - {}", host, port, err.what()));
    }
}

auto reconnect(boost::asio::io_service& io_service, const std::string& host, std::uint16_t port) ->
    std::unique_ptr<socket_type>
{
    auto socket = std::unique_ptr<socket_type>(new socket_type(io_service));
    connect<protocol_type>(io_service, *socket, host, port);

    return socket;
}

}  // namespace

tcp_t::tcp_t(std::string host, std::uint16_t port) :
    data(new data_t)
{
    data->host = std::move(host);
    data->port = port;
}

tcp_t::~tcp_t() = default;

auto tcp_t::host() const noexcept -> const std::string& {
    return data->host;
}

auto tcp_t::port() const noexcept -> std::uint16_t {
    return data->port;
}

auto tcp_t::emit(const record_t&, const string_view& message) -> void {
    std::lock_guard<std::mutex> lock(data->mutex);

    if (!data->socket) {
        data->socket = reconnect(data->io_service, data->host, data->port);
    }

    try {
        boost::asio::write(*data->socket, boost::asio::buffer(message.data(), message.size()));
    } catch (const boost::system::system_error&) {
        data->socket.reset();
        std::rethrow_exception(std::current_exception());
    }
}

}  // namespace socket
}  // namespace sink
}  // namespace v1
}  // namespace blackhole

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
