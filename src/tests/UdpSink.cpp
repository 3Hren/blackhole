#include <blackhole/sink/socket.hpp>

#include "global.hpp"
#include "mocks/socket.hpp"

using namespace blackhole;

TEST(socket_t, Class) {
    sink::socket_t<boost::asio::ip::udp> sink("localhost", 50030);
    UNUSED(sink);
}

TEST(socket_t, UdpSocketIsThreadSafe) {
    static_assert(
        sink::thread_safety<
            sink::socket_t<boost::asio::ip::udp>
        >::type::value == sink::thread::safety_t::safe,
        "`socket_t<boost::asio::ip::udp>` sink must be thread safe"
    );
}

TEST(socket_t, TestCanSendMessages) {
    sink::socket_t<
        boost::asio::ip::udp,
        NiceMock<mock::socket::backend_t>
    > sink("localhost", 50030);
    EXPECT_CALL(sink.backend(), write(std::string("formatted message"))).
            Times(1);
    sink.consume("formatted message");
}

TEST(socket_t, ThrowsExceptionOnAnyWriteErrorOccurred) {
    /*
     * This behaviour is normal for blocking udp/tcp socket sink.
     * When some network error occurs, message is on the half way to be dropped.
     * In case of UDP socket, handler will try to reopen socket immediately and
     * rewrite again here and every next message.
     * In case of TCP socket, handler will try to reconnect on every next
     * message before sending.
     */
    sink::socket_t<
        boost::asio::ip::udp,
        NiceMock<mock::socket::backend_t>
    > sink("localhost", 50030);
    EXPECT_CALL(sink.backend(), write(_))
            .Times(1)
            .WillOnce(Throw(std::exception()));
    EXPECT_THROW(sink.consume("message"), std::exception);
}

TEST(socket_t, ThrowsExceptionWhenCannotAcquireResource) {
    /*
     * This is initialization behaviour and cannot be caught by any log backend.
     * If the handler cannot acquire resource needed, it can't continue its
     * work, so it's neccessary to notify upper level code about it.
     */
    typedef sink::socket_t<
        boost::asio::ip::udp,
        NiceMock<mock::socket::failing_backend_t>
    > socket_t;
    EXPECT_THROW(socket_t("localhost", 50030), std::exception);
}

