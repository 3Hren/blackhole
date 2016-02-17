#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>
#include <blackhole/record.hpp>

#include <blackhole/detail/sink/socket/tcp.hpp>

#include "mocks/node.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace testing {

using sink::socket::tcp_t;

using ::testing::Return;
using ::testing::StrictMock;

TEST(tcp, Host) {
    EXPECT_EQ("localhost", tcp_t("localhost", 20000).host());
}

TEST(tcp, Port) {
    EXPECT_EQ(20000, tcp_t("localhost", 20000).port());
}

TEST(tcp, SendsData) {
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor(io_service,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 0));
    const auto endpoint = acceptor.local_endpoint();

    tcp_t sink(endpoint.address().to_string(), endpoint.port());

    const string_view message("");
    const attribute_pack pack;
    const record_t record(0, message, pack);

    sink.emit(record, "{}");

    boost::asio::ip::tcp::socket socket(io_service);
    acceptor.accept(socket);

    std::array<char, 2> buffer;
    boost::asio::ip::tcp::endpoint remote;
    const auto nread = boost::asio::read(socket, boost::asio::buffer(buffer),
        boost::asio::transfer_exactly(2));

    ASSERT_EQ(2, nread);
    EXPECT_EQ('{', buffer[0]);
    EXPECT_EQ('}', buffer[1]);
}

TEST(tcp, ThrowsExceptionOnConnectionRefused) {
    tcp_t sink("127.0.0.1", 1023);

    const string_view message("");
    const attribute_pack pack;
    const record_t record(0, message, pack);

    EXPECT_THROW(sink.emit(record, "{}"), std::system_error);
}

TEST(tcp_t, type) {
    EXPECT_EQ("tcp", std::string(blackhole::factory<tcp_t>::type()));
}

TEST(tcp_t, ThrowsIfHostParameterIsMissing) {
    StrictMock<config::testing::mock::node_t> config;

    EXPECT_CALL(config, subscript_key("host"))
        .Times(1)
        .WillOnce(Return(nullptr));

    try {
        factory<tcp_t>::from(config);
    } catch (const std::invalid_argument& err) {
        EXPECT_STREQ(R"(parameter "host" is required)", err.what());
    }
}

TEST(tcp_t, ThrowsIfPortParameterIsMissing) {
    using config::testing::mock::node_t;

    StrictMock<node_t> config;

    auto n1 = new node_t;
    EXPECT_CALL(config, subscript_key("host"))
        .Times(1)
        .WillOnce(Return(n1));

    EXPECT_CALL(*n1, to_string())
        .Times(1)
        .WillOnce(Return("localhost"));

    EXPECT_CALL(config, subscript_key("port"))
        .Times(1)
        .WillOnce(Return(nullptr));

    try {
        factory<tcp_t>::from(config);
    } catch (const std::invalid_argument& err) {
        EXPECT_STREQ(R"(parameter "port" is required)", err.what());
    }
}

TEST(tcp_t, Config) {
    using config::testing::mock::node_t;

    StrictMock<node_t> config;

    auto n1 = new node_t;
    EXPECT_CALL(config, subscript_key("host"))
        .Times(1)
        .WillOnce(Return(n1));

    EXPECT_CALL(*n1, to_string())
        .Times(1)
        .WillOnce(Return("0.0.0.0"));

    auto n2 = new node_t;
    EXPECT_CALL(config, subscript_key("port"))
        .Times(1)
        .WillOnce(Return(n2));

    EXPECT_CALL(*n2, to_uint64())
        .Times(1)
        .WillOnce(Return(20000));

    const auto sink = factory<tcp_t>::from(config);

    EXPECT_EQ("0.0.0.0", sink.host());
    EXPECT_EQ(20000, sink.port());
}

}  // namespace testing
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
