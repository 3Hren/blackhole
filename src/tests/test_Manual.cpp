#include "Mocks.hpp"

//#define UDP_MANUAL
//#define TCP_MANUAL

#ifdef TCP_MANUAL
TEST(socket_t, ManualTcp) {
    sink::socket_t<boost::asio::ip::tcp> sink("localhost", 50030);
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
    sink::socket_t<boost::asio::ip::udp> sink("localhost", 50030);
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

//!@todo: Simplify logger creation.
//!@todo: Make global logger.
//!@todo: Make compilable on GCC4.4.
