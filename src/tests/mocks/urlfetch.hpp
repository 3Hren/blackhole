#pragma once

#include <blackhole/sink/elasticsearch/urlfetch.hpp>

namespace testing {

namespace mock {

class stream_t {
public:
    stream_t(boost::asio::io_service&) {}

    MOCK_METHOD2(
        async_open,
        void(
            const urdl::url&,
            std::function<void(const boost::system::error_code&)>
        )
    );

    MOCK_METHOD2(
        async_read_some,
        void(
            boost::asio::mutable_buffer,
            std::function<void(const boost::system::error_code&, std::size_t)>
        )
    );

    MOCK_METHOD0(close, void());

    MOCK_METHOD1(set_options, void(const urdl::option_set&));
    MOCK_CONST_METHOD0(is_open, bool());
};

} // namespace mock

} // namespace testing
