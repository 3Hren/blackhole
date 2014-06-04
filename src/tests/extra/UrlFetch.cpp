#include <array>

#include <boost/asio.hpp>

#include <urdl/option_set.hpp>
#include <urdl/read_stream.hpp>

#include <blackhole/utils/atomic.hpp>

#include "../global.hpp"

namespace urlfetch {

struct request_t {
    std::string url;
    urdl::option_set options;
    long timeout;
};

struct response_t {
    std::string data;
};

template<class Stream = urdl::read_stream>
class task_t : public std::enable_shared_from_this<task_t<Stream>> {
public:
    typedef Stream stream_type;
    typedef boost::asio::io_service loop_type;
    typedef std::function<
        void(request_t&&, response_t&&, const boost::system::error_code&)
    > callback_type;

private:
    std::array<char, 4096> buffer;

    request_t request;
    response_t response;
    callback_type callback;
    stream_type stream_;

public:
    task_t(request_t request, callback_type callback, loop_type& loop) :
        request(std::move(request)),
        callback(std::move(callback)),
        stream_(loop)
    {}

    stream_type& stream() {
        return stream_;
    }

    void run() {
        //!@todo: do nothing if the stream is active.
        stream_.async_open(
            request.url,
            std::bind(
                &task_t::on_open,
                this->shared_from_this(),
                std::placeholders::_1
            )
        );
    }

private:
    void on_open(const boost::system::error_code& ec) {
        std::cout << "on_open" << std::endl;
        //!@todo:
//        if (ec) {
//            callback(url, "", ec);
//            return;
//        }

        post_read();
    }

    void on_read(const boost::system::error_code& ec, std::size_t length) {
        std::cout << "on_read" << std::endl;
        if (ec) {
            //!@todo: timer.cancel();
            if (ec.value() == boost::asio::error::eof) {
                callback(
                    std::move(request),
                    std::move(response),
                    boost::system::error_code()
                );
            } else {
                //!@todo:
//                callback(
//                    std::move(request),
//                    std::move(response),
//                    ec
//                );
            }

            return;
        }

        response.data.append(buffer.begin(), buffer.begin() + length);
        post_read();
    }

    void post_read() {
        stream_.async_read_some(
            boost::asio::buffer(buffer),
            std::bind(
                &task_t::on_read,
                this->shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2
            )
        );
    }
};

} // namespace urlfetch

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
};

} // namespace mock

TEST(request_t, Class) {
    urlfetch::request_t request;
    UNUSED(request);
}

TEST(urlfetch_t, Class) {
    urlfetch::request_t request;
    urlfetch::task_t<mock::stream_t>::loop_type loop;
    urlfetch::task_t<mock::stream_t>::callback_type callback;
    urlfetch::task_t<mock::stream_t> task(request, callback, loop);
    UNUSED(task);
}

void post_open(boost::asio::io_service& loop,
               std::function<void(const boost::system::error_code&)> task,
               const boost::system::error_code& ec) {
    loop.post(std::bind(task, ec));
}

void post_read(boost::asio::io_service& loop,
               const boost::asio::mutable_buffer& buffer,
               std::function<
                   void(const boost::system::error_code&, std::size_t length)
               > task,
               const boost::system::error_code& ec,
               std::string data) {
    BOOST_ASSERT(data.size() <= 4096);

    std::size_t size = boost::asio::buffer_size(buffer);
    EXPECT_EQ(4096, size);

    unsigned char* p = boost::asio::buffer_cast<unsigned char*>(buffer);
    std::copy(data.begin(), data.end(), p);
    loop.post(std::bind(task, ec, data.size()));
}

namespace testing {

struct event_t {
    std::atomic<int>& counter;
};

struct successfull_get_t {
    std::atomic<int>& counter;

    void operator()(urlfetch::request_t&& request,
                    urlfetch::response_t&& response,
                    const boost::system::error_code& ec) {
        EXPECT_EQ("http://127.0.0.1:80", request.url);
        EXPECT_EQ("{}", response.data);
        EXPECT_EQ(0, ec.value());
        counter++;
    }
};

} // namespace testing

TEST(urlfetch_t, SuccessfullGet) {
    urlfetch::request_t request;
    request.url = "http://127.0.0.1:80";

    std::atomic<int> counter(0);
    testing::successfull_get_t event { counter };
    urlfetch::task_t<mock::stream_t>::loop_type loop;
    auto task = std::make_shared<
        urlfetch::task_t<mock::stream_t>
    >(request, event, loop);

    EXPECT_CALL(task->stream(), async_open(urdl::url("http://127.0.0.1:80"), _))
            .Times(1)
            .WillOnce(
                WithArg<1>(
                    Invoke(
                        std::bind(
                            &post_open,
                            std::ref(loop),
                            std::placeholders::_1,
                            boost::system::error_code()
                        )
                    )
                )
            );
    task->run();

    // First read event - just writing some bytes to the buffer.
    EXPECT_CALL(task->stream(), async_read_some(_, _))
            .Times(1)
            .WillOnce(
                Invoke(
                    std::bind(
                        &post_read,
                        std::ref(loop),
                        std::placeholders::_1,
                        std::placeholders::_2,
                        boost::system::error_code(),
                        "{}"
                    )
                )
            );

    // Poll `on_open` task.
    loop.run_one();

    // Second read event - pushing EOF "error" to notify, that no more bytes
    // will be received.
    EXPECT_CALL(task->stream(), async_read_some(_, _))
            .Times(1)
            .WillOnce(
                Invoke(
                    std::bind(
                        &post_read,
                        std::ref(loop),
                        std::placeholders::_1,
                        std::placeholders::_2,
                        boost::asio::error::make_error_code(
                            boost::asio::error::eof
                        ),
                        ""
                    )
                )
            );

    // Pool `on_read` first task.
    loop.run_one();

    // Poll `on_read` second task.
    loop.run_one();

    EXPECT_EQ(1, counter);
}

// Test-> failed
// Test-> timeout
// Test-> cancel

TEST(urlfetch_t, Manual) {
//    urlfetch::get(url, callback, loop, timeout=default);
//    urlfetch::post(url, body, callback, loop, timeout=default);
//    urlfetch::execute(request, callback, loop);
}
