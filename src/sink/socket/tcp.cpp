#include <mutex>

#include <boost/asio/write.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/stdext/string_view.hpp"
#include "blackhole/extensions/format.hpp"
#include "blackhole/sink/socket/tcp.hpp"

#include "blackhole/detail/memory.hpp"
#include "blackhole/detail/util/optional.hpp"

#include "tcp.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace socket {

typedef boost::asio::ip::tcp protocol_type;
typedef protocol_type::socket socket_type;
typedef protocol_type::endpoint endpoint_type;

namespace {

template<typename Protocol, typename SocketService, typename Iterator>
auto do_connect(boost::asio::basic_socket<Protocol, SocketService>& s,
                Iterator begin,
                Iterator end,
                boost::system::error_code& ec) -> Iterator
{
    ec = boost::system::error_code();

    for (Iterator iter = begin; iter != end; ++iter) {
        if (iter != end) {
            s.close(ec);
            s.connect(*iter, ec);
            if (!ec) {
                return iter;
            }
        }
    }

    if (!ec) {
        ec = boost::asio::error::not_found;
    }

    return end;
}

template <typename Protocol, typename SocketService, typename Iterator>
auto do_connect(boost::asio::basic_socket<Protocol, SocketService>& s, Iterator begin) -> Iterator {
    boost::system::error_code ec;
    Iterator end = typename Protocol::resolver::iterator();
    Iterator result = do_connect(s, begin, end, ec);
    boost::asio::detail::throw_error(ec);
    return result;
}

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
        do_connect(socket, endpoint);
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
    host_(std::move(host)),
    port_(port)
{}

auto tcp_t::host() const noexcept -> const std::string& {
    return host_;
}

auto tcp_t::port() const noexcept -> std::uint16_t {
    return port_;
}

auto tcp_t::emit(const record_t&, const string_view& message) -> void {
    std::lock_guard<std::mutex> lock(mutex);

    if (!socket) {
        socket = reconnect(io_service, host(), port());
    }

    try {
        boost::asio::write(*socket, boost::asio::buffer(message.data(), message.size()));
    } catch (const boost::system::system_error&) {
        socket.reset();
        std::rethrow_exception(std::current_exception());
    }
}

}  // namespace socket
}  // namespace sink

using sink::socket::tcp_t;

using detail::util::value_or;

auto factory<tcp_t>::type() const noexcept -> const char* {
    return "tcp";
}

auto factory<tcp_t>::from(const config::node_t& config) const -> std::unique_ptr<sink_t> {
    const auto host = value_or(config["host"].to_string(), []() -> std::string {
        throw std::invalid_argument(R"(parameter "host" is required)");
    });

    const auto port = value_or(config["port"].to_uint64(), []() -> std::uint64_t {
        throw std::invalid_argument(R"(parameter "port" is required)");
    });

    return blackhole::make_unique<tcp_t>(host, static_cast<std::uint16_t>(port));
}

}  // namespace v1
}  // namespace blackhole
