#include "Mocks.hpp"

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

namespace asio = boost::asio;

namespace blackhole {

namespace sink {

//! Resolves specified host and tries to connect to the socket.
template<typename Protocol>
void
connect(asio::io_service& io_service, typename Protocol::socket& socket, const std::string& host, std::uint16_t port) {
    try {
        typename Protocol::resolver resolver(io_service);
        typename Protocol::resolver::query query(host, boost::lexical_cast<std::string>(port));
        typename Protocol::resolver::iterator it = resolver.resolve(query);

        try {
            asio::connect(socket, it);
        } catch (const boost::system::system_error& err) {
            throw error_t("couldn't connect to the %s:%d - %s", host, port, err.what());
        }
    } catch (const boost::system::system_error& err) {
        throw error_t("couldn't resolve %s:%d - %s", host, port, err.what());
    }
}

//template<typename Protocol>
class boost_asio_backend_t {
    typedef boost::asio::ip::udp Protocol;

    const std::string host;
    const std::uint16_t port;

    boost::asio::io_service io_service;
    Protocol::socket socket;

public:
    boost_asio_backend_t(const std::string& host, std::uint16_t port) :
        host(host),
        port(port),
        socket(io_service)
    {
        connect<Protocol>(io_service, socket, host, port);
    }

    ssize_t write(const std::string& message) {
        return socket.send(boost::asio::buffer(message.data(), message.size()));
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
    const std::string host;
    const std::uint16_t port;

    asio::io_service io_service;
    std::unique_ptr<asio::ip::tcp::socket> socket;

public:
    tcp_socket_t(const std::string& host, std::uint16_t port) :
        host(host),
        port(port),
        socket(initialize(io_service, host, port))
    {
    }

    void consume(const std::string& message) {
        if (!socket) {
            socket = initialize(io_service, host, port);
        }

        try {
            socket->write_some(boost::asio::buffer(message.data(), message.size()));
        } catch (const boost::system::system_error& err) {
            socket.release();
            std::rethrow_exception(std::current_exception());
        }
    }

private:
    static inline
    std::unique_ptr<asio::ip::tcp::socket>
    initialize(asio::io_service& io_service, const std::string& host, std::uint16_t port) {
        std::unique_ptr<asio::ip::tcp::socket> socket = std::make_unique<asio::ip::tcp::socket>(io_service);
        connect(io_service, *socket, host, port);
        return socket;
    }

    static inline
    void
    connect(asio::io_service& io_service, asio::ip::tcp::socket& socket, const std::string& host, std::uint16_t port) {
        try {
            asio::ip::tcp::resolver resolver(io_service);
            asio::ip::tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
            asio::ip::tcp::resolver::iterator it = resolver.resolve(query);

            try {
                asio::connect(socket, it);
            } catch (const boost::system::system_error& err) {
                throw error_t("couldn't connect to the %s:%d - %s", host, port, err.what());
            }
        } catch (const boost::system::system_error& err) {
            throw error_t("couldn't resolve %s:%d - %s", host, port, err.what());
        }
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
    // This behaviour is normal for blocking udp/tcp socket sink.
    // When some network error occurs, message is on the half way to be dropped.
    // In case of UDP socket, handler will try to reopen socket immediately and rewrite again here
    // and every next message.
    // In case of TCP socket, handler will try to reconnect on every next message before sending.
    sink::socket_t<NiceMock<mock::socket::backend_t>> sink("localhost", 50030);
    EXPECT_CALL(sink.backend(), write(_))
            .Times(1)
            .WillOnce(Throw(std::exception()));
    EXPECT_THROW(sink.consume("message"), std::exception);
}

TEST(socket_t, ThrowsExceptionWhenCannotAcquireResource) {
    // This is initialization behaviour and cannot be caught by any log backend. If handler cannot
    // acquire resource needed, it can't continue its work, so it's neccessary to notify upper level
    // code about it.
    EXPECT_THROW(sink::socket_t<NiceMock<mock::socket::failing_backend_t>>("localhost", 50030), std::exception); //!@todo: Maybe some kind of typecheck here?
}

#define UDP_MANUAL

#ifdef TCP_MANUAL
TEST(socket_t, ManualTcp) {
    sink::tcp_socket_t sink("localhost", 50030);
    int i = 0;
    while (true) {
        try {
            sink.consume(utils::format("{\"@message\": \"value = %d\"}\n", i));
        } catch (std::exception& e) {
            std::cout << utils::format("I've fucked up: %s", e.what()) << std::endl;
        }

        i++;
        usleep(1000000);
    }
}
#endif

#ifdef UDP_MANUAL
TEST(socket_t, ManualUdp) {
    sink::socket_t<> sink("localhost", 50030);
    int i = 0;
    while (true) {
        try {
            sink.consume(utils::format("{\"@message\": \"value = %d\"}\n", i));
        } catch (std::exception& e) {
            std::cout << utils::format("I've fucked up: %s", e.what()) << std::endl;
        }

        i++;
        usleep(1000000);
    }
}
#endif
