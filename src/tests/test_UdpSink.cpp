#include "Mocks.hpp"

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

namespace asio = boost::asio;

namespace blackhole {

namespace sink {

class boost_asio_backend_t {
    asio::io_service io_service;
    asio::ip::udp::endpoint endpoint;
    std::unique_ptr<asio::ip::udp::socket> socket;

public:
    boost_asio_backend_t(const std::string& host, std::uint16_t port) {
        asio::ip::udp::resolver resolver(io_service);
        asio::ip::udp::resolver::query query(host, boost::lexical_cast<std::string>(port));
        asio::ip::udp::resolver::iterator it = resolver.resolve(query); //!@todo: May throw!
        std::vector<asio::ip::udp::endpoint> endpoints(it, asio::ip::udp::resolver::iterator());

        for (auto it = endpoints.begin(); it != endpoints.end(); ++it) {
            try {
                endpoint = *it;
                socket = std::make_unique<asio::ip::udp::socket>(io_service);
                socket->open(endpoint.protocol());
                break;
            } catch (const boost::system::system_error& err) {
                std::cout << err.what() << std::endl;
                continue;
            }
        }

        if (!socket) {
            throw error_t("can not create socket");
        }
    }

    ssize_t write(const std::string& message) {
        return socket->send_to(boost::asio::buffer(message.data(), message.size()), endpoint);
    }
};

template<typename Backend = boost_asio_backend_t>
class socket_t {
    Backend m_backend;

public:
    socket_t(const std::string& host, std::uint16_t port) :
        m_backend(host, port)
    {
    }

    void consume(const std::string& message) {
        m_backend.write(message);
    }

    Backend& backend() {
        return m_backend;
    }
};

class tcp_socket_t {
    asio::io_service io_service;
    asio::ip::tcp::endpoint endpoint;
    std::unique_ptr<asio::ip::tcp::socket> socket;

public:
    tcp_socket_t() {
        asio::ip::tcp::resolver resolver(io_service);
        asio::ip::tcp::resolver::query query("localhost", boost::lexical_cast<std::string>(50030));
        asio::ip::tcp::resolver::iterator it = resolver.resolve(query); //!@todo: May throw!
        std::vector<asio::ip::tcp::endpoint> endpoints(it, asio::ip::tcp::resolver::iterator());

        for (auto it = endpoints.begin(); it != endpoints.end(); ++it) {
            try {
                endpoint = *it;
                socket = std::make_unique<asio::ip::tcp::socket>(io_service);
                socket->connect(endpoint);
                break;
            } catch (const boost::system::system_error& err) {
                std::cout << err.what() << std::endl;
                continue;
            }
        }

        if (!socket) {
            throw error_t("can not create socket");
        }
        std::cout << endpoint << std::endl;
    }

    void consume(const std::string& message) {
         auto written = socket->write_some(boost::asio::buffer(message.data(), message.size()));
         std::cout << utils::format("written: %d, available: %d", written, socket->available()) << std::endl;
    }
};

} // namespace sink

} // namespace blackhole

TEST(socket_t, Class) {
    sink::socket_t<> sink("localhost", 50030);
    UNUSED(sink);
}

TEST(socket_t, TestCanSendMessages) {
    sink::socket_t<NiceMock<mock::socket::backend_t>> sink("localhost", 50030);
    EXPECT_CALL(sink.backend(), write(std::string("formatted message"))).
            Times(1);
    sink.consume("formatted message");
}

TEST(socket_t, ThrowsExceptionOnAnyWriteErrorOccurred) {
    sink::socket_t<NiceMock<mock::socket::backend_t>> sink("localhost", 50030);
    EXPECT_CALL(sink.backend(), write(_))
            .Times(1)
            .WillOnce(Throw(std::exception()));
    EXPECT_THROW(sink.consume("message"), std::exception);
}

//!@todo: DoSomethingIfCannotCreateSocket
//!@todo: DoSomethingIfCannotConnect
//!@todo: DoSomethingOnWriteError
