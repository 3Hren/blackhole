#include "Mocks.hpp"

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

namespace asio = boost::asio;

namespace blackhole {

namespace sink {

class socket_t {
    asio::io_service io_service;
    asio::ip::udp::endpoint endpoint;
    std::unique_ptr<asio::ip::udp::socket> socket;

public:
    socket_t(const std::string& host, std::uint16_t port)
    {
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

    void consume(const std::string& message) {
        try {
            socket->send_to(boost::asio::buffer(message.data(), message.size()), endpoint);
        } catch (const std::exception& err) {
            std::cout << "!" << err.what() << std::endl;
        } catch (...) {
            std::cout << "WTF?" << std::endl;
        }
    }
};

} // namespace sink

} // namespace blackhole

TEST(socket_t, Class) {
    sink::socket_t sink("localhost", 50030);
    sink.consume("{\"@message\": \"le message\"}");
}

//!@todo: TestCanSendMessages.
//!@todo: ThrowsExceptionIfAnyErrorOccurred.
//!@todo: DoSomethingIfCannotCreateSocket
//!@todo: DoSomethingIfCannotConnect
//!@todo: DoSomethingOnWriteError
