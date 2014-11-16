#include <boost/asio.hpp>

#include <blackhole/sink/elasticsearch/resolver.hpp>

#include "../global.hpp"

using elasticsearch::resolver;

typedef boost::asio::ip::tcp protocol_type;
typedef protocol_type::endpoint endpoint_type;

boost::asio::ip::address_v4::bytes_type localhost_v4 = {{ 127, 0, 0, 1 }};
boost::asio::ip::address_v6::bytes_type localhost_v6 = {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }};

TEST(resolver, FullResolveByIp) {
    boost::asio::io_service service;
    EXPECT_EQ(
        endpoint_type(boost::asio::ip::address_v4(localhost_v4), 5000),
        resolver<protocol_type>::resolve("localhost/127.0.0.1:5000", service)
    );
}

TEST(resolver, ResolveByIp) {
    boost::asio::io_service service;
    EXPECT_EQ(
        endpoint_type(boost::asio::ip::address_v4(localhost_v4), 5000),
        resolver<protocol_type>::resolve("/127.0.0.1:5000", service)
    );
}

TEST(resolver, ResolveByIpWithoutSlash) {
    boost::asio::io_service service;
    EXPECT_EQ(
        endpoint_type(boost::asio::ip::address_v4(localhost_v4), 5000),
        resolver<protocol_type>::resolve("127.0.0.1:5000", service)
    );
}

TEST(resolver, ResolveByHostname) {
    boost::asio::io_service service;
    resolver<protocol_type>::resolve("localhost/:5000", service);
    auto expected_v4 = endpoint_type(
        boost::asio::ip::address_v4(localhost_v4),
        5000
    );

    auto expected_v6 = endpoint_type(
        boost::asio::ip::address_v6(localhost_v6),
        5000
    );

    auto actual = resolver<protocol_type>::resolve("localhost/:5000", service);

    // Expanded fe80:1::1.
    auto isatap = endpoint_type(
        boost::asio::ip::address_v6(boost::asio::ip::address_v6::bytes_type {{
            254, 128, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
        }}),
        5000
    );

    EXPECT_THAT(actual, AnyOf(
        Eq(expected_v4),
        Eq(expected_v6),
        Eq(isatap)
    ));
}

TEST(resolver, ThrowExceptionWhenPortNotSpecifyed) {
    boost::asio::io_service service;

    EXPECT_THROW(
        resolver<protocol_type>::resolve("localhost/127.0.0.1", service),
        std::logic_error
    );
}

TEST(resolver, ThrowExceptionWhenIncorrectInput) {
    boost::asio::io_service service;

    EXPECT_THROW(
        resolver<protocol_type>::resolve("localhost//127.0.0.1:5000", service),
        std::runtime_error
    );
}
