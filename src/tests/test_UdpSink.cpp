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
        asio::ip::udp::resolver::iterator end;
        std::vector<asio::ip::udp::endpoint> endpoints(it, end);

        for (auto it = endpoints.begin(); it != endpoints.end(); ++it) {
            try {
                socket = std::make_unique<asio::ip::udp::socket>(io_service);
                endpoint = *it;
                socket->open(endpoint.protocol());
                break;
            } catch (const boost::system::system_error& err) {
                std::cout << err.what() << std::endl;
                continue;
            }
        }
    }

    ssize_t write(const std::string& message) {
        return socket->send_to(boost::asio::buffer(message.data(), message.size()), endpoint);
    }
};

template<typename Backend>
class socket_t {
    Backend m_backend;

public:
    socket_t(const std::string& host, std::uint16_t port) :
        m_backend(host, port)
    {
    }

    void consume(const std::string& message) {
        try {
            m_backend.write(message);
        } catch (const std::exception& err) {
            std::cout << err.what() << std::endl;
        } catch (...) {
            std::cout << "unknown error occurred while writing message to the socket" << std::endl;
        }
    }

    Backend& backend() {
        return m_backend;
    }
};

} // namespace sink

} // namespace blackhole

TEST(socket_t, TestCanSendMessages) {
    sink::socket_t<NiceMock<mock::socket::backend_t>> sink("localhost", 50030);
    EXPECT_CALL(sink.backend(), write(std::string("formatted message"))).
            Times(1);
    sink.consume("formatted message");
}

//!@todo: ThrowsExceptionIfAnyErrorOccurred.
//!@todo: DoSomethingIfCannotCreateSocket
//!@todo: DoSomethingIfCannotConnect
//!@todo: DoSomethingOnWriteError
