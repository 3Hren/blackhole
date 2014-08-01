#include <blackhole/sink/socket.hpp>

#include "mocks.hpp"

using namespace blackhole;

TEST(socket_t, TcpSocketIsThreadUnsafe) {
    static_assert(
        sink::thread_safety<
            sink::socket_t<boost::asio::ip::tcp>
        >::type::value == sink::thread::safety_t::unsafe,
        "`socket_t<boost::asio::ip::tcp>` sink must be thread unsafe"
    );
}
