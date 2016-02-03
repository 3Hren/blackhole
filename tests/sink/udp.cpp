#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/asio/ip/udp.hpp>

#include <blackhole/attribute.hpp>
#include <blackhole/record.hpp>
#include <blackhole/sink/socket/udp.hpp>

namespace blackhole {
namespace testing {
namespace sink {

using blackhole::sink::socket::udp_t;

TEST(udp_t, Endpoint) {
    udp_t sink("0.0.0.0", 20000);

    EXPECT_EQ(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 20000), sink.endpoint());
}

TEST(udp_t, SendsData) {
    boost::asio::io_service io_service;
    boost::asio::ip::udp::socket socket(io_service,
        boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));
    const auto endpoint = socket.local_endpoint();

    udp_t sink(endpoint.address().to_string(), endpoint.port());

    const string_view message("");
    const attribute_pack pack;
    const record_t record(0, message, pack);

    sink.emit(record, "{}");

    std::array<char, 2> buffer;
    boost::asio::ip::udp::endpoint remote;
    socket.receive_from(boost::asio::buffer(buffer), remote, 0);

    EXPECT_EQ('{', buffer[0]);
    EXPECT_EQ('}', buffer[1]);
}

TEST(udp_t, type) {
    EXPECT_EQ("udp", std::string(blackhole::factory<udp_t>::type()));
}

}  // namespace sink
}  // namespace testing
}  // namespace blackhole
