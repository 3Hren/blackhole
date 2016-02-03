#include "blackhole/sink/socket/udp.hpp"

#include <boost/asio/ip/udp.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace socket {

namespace udp {

class inner_t {
public:
    virtual ~inner_t() {}
    virtual auto write(const string_view& data) -> void = 0;
    virtual auto endpoint() const -> const boost::asio::ip::udp::endpoint& = 0;
};

class blocking_t : public inner_t {
    boost::asio::io_service io_service;
    boost::asio::ip::udp::socket socket;
    boost::asio::ip::udp::endpoint endpoint_;

public:
    blocking_t(const std::string& host, std::uint16_t port) :
        io_service(),
        socket(io_service)
    {
        boost::asio::ip::udp::resolver resolver(io_service);
        boost::asio::ip::udp::resolver::query query(host, boost::lexical_cast<std::string>(port),
            boost::asio::ip::udp::resolver::query::flags::numeric_service);
        endpoint_ = *resolver.resolve(query);

        socket.open(endpoint_.protocol());
    }

    auto write(const string_view& data) -> void {
        socket.send_to(boost::asio::buffer(data.data(), data.size()), endpoint_);
    }

    auto endpoint() const -> const boost::asio::ip::udp::endpoint& {
        return endpoint_;
    }
};

class nonblocking_t : public inner_t {
public:
};

}  // namespace udp

udp_t::udp_t(const std::string& host, std::uint16_t port) :
    inner(new udp::blocking_t(host, port))
{}

udp_t::~udp_t() = default;

auto udp_t::endpoint() const -> const endpoint_type& {
    return inner->endpoint();
}

auto udp_t::filter(const record_t&) -> bool {
    return true;
}

auto udp_t::emit(const record_t&, const string_view& formatted) -> void {
    inner->write(formatted);
}

}  // namespace socket
}  // namespace sink

auto factory<sink::socket::udp_t>::type() -> const char* {
    return "udp";
}

namespace {

template<typename T>
auto expect(const boost::optional<T>& optional, const std::string& description) -> T {
    if (optional) {
        return optional.get();
    } else {
        throw std::invalid_argument(description);
    }
}

}  // namespace

auto factory<sink::socket::udp_t>::from(const config::node_t& config) -> sink::socket::udp_t {
    const auto host = expect(config["host"].to_string(), "parameter \"host\" is required");
    const auto port = expect(config["port"].to_uint64(), "parameter \"port\" is required");

    return {host, static_cast<std::uint16_t>(port)};
}

}  // namespace v1
}  // namespace blackhole
