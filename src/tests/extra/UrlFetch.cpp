#include <array>

#include <boost/asio.hpp>
#include <boost/optional.hpp>

#include <urdl/option_set.hpp>
#include <urdl/read_stream.hpp>

#include <blackhole/utils/atomic.hpp>

#include "../global.hpp"

namespace urlfetch {

struct request_t {
    std::string url;
    urdl::option_set options;
    boost::posix_time::milliseconds timeout;

    request_t() : timeout(1000) {}
};

struct response_t {
    std::string data;
};

template<class Stream = urdl::read_stream>
class task_t : public std::enable_shared_from_this<task_t<Stream>> {
public:
    typedef Stream stream_type;
    typedef boost::asio::io_service loop_type;
    typedef boost::posix_time::milliseconds timeout_type;
    typedef std::function<
        void(request_t&&, response_t&&, const boost::system::error_code&)
    > callback_type;

private:
    char buffer[4096];

    request_t request;
    response_t response;
    callback_type callback;

    stream_type stream_;
    boost::asio::deadline_timer timer;

public:
    task_t(request_t request, callback_type callback, loop_type& loop) :
        request(std::move(request)),
        callback(std::move(callback)),
        stream_(loop),
        timer(loop)
    {
        response.data.reserve(16384);
    }

    stream_type& stream() {
        return stream_;
    }

    void run() {
        //!@todo: do nothing if the stream is active.
        stream_.set_options(request.options);
        stream_.async_open(
            request.url,
            std::bind(
                &task_t::on_open,
                this->shared_from_this(),
                std::placeholders::_1
            )
        );

        timer.expires_from_now(request.timeout);
        timer.async_wait(
            std::bind(
                &task_t::on_timeout,
                this->shared_from_this(),
                std::placeholders::_1
            )
        );
    }

    void cancel() {
        stream_.close();
    }

private:
    void on_open(const boost::system::error_code& ec) {
        if (ec) {
            timer.cancel();
            callback(std::move(request), std::move(response), ec);
            return;
        }

        async_read_some();
    }

    void on_read(const boost::system::error_code& ec, std::size_t length) {
        response.data.append(buffer, length);
        if (ec) {
            timer.cancel();
            callback(
                std::move(request),
                std::move(response),
                make_error_code(ec)
            );
            return;
        }

        async_read_some();
    }

    void on_timeout(const boost::system::error_code& ec) {
        if (!ec) {
            stream_.close();
        }
    }

    boost::system::error_code
    make_error_code(const boost::system::error_code& ec) const {
        switch (ec.value()) {
        case boost::asio::error::eof:
            return boost::system::error_code();
        case boost::asio::error::operation_aborted:
            if (timer.expires_from_now() > boost::posix_time::time_duration()) {
                return ec;
            } else {
                return boost::asio::error::make_error_code(
                    boost::asio::error::timed_out
                );
            }
        default:
            return ec;
        }

        return ec;
    }

    void async_read_some() {
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

    MOCK_METHOD0(close, void());
    MOCK_METHOD1(set_options, void(const urdl::option_set&));
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

TEST(urlfetch_t, SuccessfulGet) {
    //!\brief GET request, which should handle correctly.

    /*! The main idea of this test is to check http stream's correct method
     *  invocation sequence.
     *  We create GET request and push it into the urlfetch::task_t object.
     *  The first event should be a connection opening, from which our mock
     *  must be called, which puts stream's read callback invocation with
     *  correct error code (no errors) into the event loop.
     *  Then we poll one event which should be exactly that callback.
     *  We mock it to write empty json string "{}" into the buffer and then
     *  just wait for the next chunk.
     *  The next event loop tick there will be no chunks, but there will be EOF
     *  error, which should trigger result callback's invocation with all
     *  received data.
     */
    urlfetch::request_t request;
    request.url = "http://127.0.0.1:80";

    std::atomic<int> counter(0);
    testing::successfull_get_t event { counter };
    urlfetch::task_t<mock::stream_t>::loop_type loop;
    auto task = std::make_shared<
        urlfetch::task_t<mock::stream_t>
    >(request, event, loop);

    EXPECT_CALL(task->stream(), set_options(_));
    // We mock `async_open` to handle correctly and post channel reading.
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

    // The first read event - just writing some bytes to the buffer.
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

    // The second read event - pushing EOF "error" to notify, that there will
    // no more bytes be received.
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

TEST(urlfetch_t, SuccessfulPost) {
    //!@todo: Implement.
}

namespace testing {

struct connection_error_t {
    std::atomic<int>& counter;
    const boost::system::error_code ec;

    void operator()(urlfetch::request_t&&,
                    urlfetch::response_t&&,
                    const boost::system::error_code& ec) {
        EXPECT_EQ(this->ec.value(), ec.value());
        counter++;
    }
};

} // namespace testing

TEST(urlfetch_t, DirectConnectionError) {
    //!\brief GET request, which should fail with connection error on opening.

    /*! We manually set connection refused error when the stream tries to open
     *  connection. Entire request should fail, so there should be expected
     *  callback invocation with that error.
     */
    std::atomic<int> counter(0);

    urlfetch::request_t request;
    request.url = "http://127.0.0.1:80";

    const auto expected = boost::asio::error::make_error_code(
        boost::asio::error::connection_refused
    );

    testing::connection_error_t event { counter, expected };
    urlfetch::task_t<mock::stream_t>::loop_type loop;
    auto task = std::make_shared<
        urlfetch::task_t<mock::stream_t>
    >(request, event, loop);

    EXPECT_CALL(task->stream(), set_options(_));

    // We mock `async_open` to emit connection error.
    EXPECT_CALL(task->stream(), async_open(_, _))
            .Times(1)
            .WillOnce(
                WithArg<1>(
                    Invoke(
                        std::bind(
                            &post_open,
                            std::ref(loop),
                            std::placeholders::_1,
                            expected
                        )
                    )
                )
            );
    task->run();

    // Extract callback fromt the event loop.
    loop.run_one();
    EXPECT_EQ(1, counter);
}

TEST(urlfetch_t, DeferredConnectionError) {
    //!\brief GET request, which should fail with connection error on reading.

    /*! A connection should successfully be opened without errors, but then we
     *  manually set connection refused error when the stream receives read
     *  event. Entire request should fail, so there should be expected callback
     *  invocation with that error.
     */
    std::atomic<int> counter(0);

    urlfetch::request_t request;
    request.url = "http://127.0.0.1:80";

    const auto expected = boost::asio::error::make_error_code(
        boost::asio::error::broken_pipe
    );
    testing::connection_error_t event { counter, expected };
    urlfetch::task_t<mock::stream_t>::loop_type loop;
    auto task = std::make_shared<
        urlfetch::task_t<mock::stream_t>
    >(request, event, loop);

    EXPECT_CALL(task->stream(), set_options(_));

    // We mock `async_open` to handle correctly and post channel reading.
    EXPECT_CALL(task->stream(), async_open(_, _))
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

    // The first read event results in broken pipe error.
    EXPECT_CALL(task->stream(), async_read_some(_, _))
            .Times(1)
            .WillOnce(
                Invoke(
                    std::bind(
                        &post_read,
                        std::ref(loop),
                        std::placeholders::_1,
                        std::placeholders::_2,
                        expected,
                        ""
                    )
                )
            );

    // Extract open event from the event loop.
    loop.run_one();

    // Extract callback.
    loop.run_one();

    EXPECT_EQ(1, counter);
}

namespace testing {

typedef std::function<
    void(const boost::system::error_code&, std::size_t length)
> task_type;

void assign(boost::optional<task_type>& lhs, task_type rhs) {
    lhs = rhs;
}

void apply(boost::optional<task_type>& task,
           boost::asio::io_service& loop,
           const boost::system::error_code& ec) {
    ASSERT_TRUE(task.is_initialized());
    loop.post(std::bind(task.get(), ec, 0));
}

} // namespace testing

TEST(urlfetch_t, Timeout) {
    //! \brief Test timeout event.

    /*! By default every request has 1000 milliseconds timeout. We set it to
     *  10 ms and create GET task. After successful stream opening we just do
     *  nothing, imitating by that long delay, which should be interrupted
     *  by the timeout event.
     *  Entire request should fail with timed out error.
     */

    std::atomic<int> counter(0);

    urlfetch::request_t request;
    request.url = "http://127.0.0.1:80";
    request.timeout = boost::posix_time::milliseconds(10);

    const auto expected = boost::asio::error::make_error_code(
        boost::asio::error::timed_out
    );
    testing::connection_error_t event { counter, expected };
    urlfetch::task_t<mock::stream_t>::loop_type loop;
    auto task = std::make_shared<
        urlfetch::task_t<mock::stream_t>
    >(request, event, loop);

    EXPECT_CALL(task->stream(), set_options(_));

    // We mock `async_open` to handle correctly and post channel reading.
    EXPECT_CALL(task->stream(), async_open(_, _))
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

    // The first read event results in nothing. There will be no `post_read`
    // method invocation, so the only way to stop the event loop - is to wait
    // for timeout. Thus, we must save callback into some variable to be able
    // to invoke it later.
    boost::optional<testing::task_type> saved_task;
    EXPECT_CALL(task->stream(), async_read_some(_, _))
            .Times(1)
            .WillOnce(
                WithArg<1>(
                    Invoke(
                        std::bind(
                            testing::assign,
                            std::ref(saved_task),
                            std::placeholders::_1
                        )
                    )
                )
            );

    // Extract open event.
    loop.run_one();

    // Expected that the next event will be the timer event. We just call
    // previously saved `on_read` callback with operation aborted error to
    // imitate real stream behaviour.
    EXPECT_CALL(task->stream(), close())
            .Times(1)
            .WillOnce(
                Invoke(
                    std::bind(
                        testing::apply,
                        std::ref(saved_task),
                        std::ref(loop),
                        boost::asio::error::make_error_code(
                            boost::asio::error::operation_aborted
                        )
                    )
                )
            );

    // Extract timeout event.
    loop.run_one();

    // Extract posted `on_read` event with aborted operation.
    loop.run_one();

    EXPECT_EQ(1, counter);
}

TEST(urlfetch_t, Cancel) {
    std::atomic<int> counter(0);

    urlfetch::request_t request;
    request.url = "http://127.0.0.1:80";

    const auto expected = boost::asio::error::make_error_code(
        boost::asio::error::operation_aborted
    );
    testing::connection_error_t event { counter, expected };
    urlfetch::task_t<mock::stream_t>::loop_type loop;
    auto task = std::make_shared<
        urlfetch::task_t<mock::stream_t>
    >(request, event, loop);

    EXPECT_CALL(task->stream(), set_options(_));

    // We mock `async_open` to handle correctly and post channel reading.
    EXPECT_CALL(task->stream(), async_open(_, _))
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

    // The first read event results in nothing. There will be no `post_read`
    // method invocation, so there are two ways to stop the event loop -
    // is to wait for timeout or to cancel. Thus, we must save callback into
    // some variable to be able to invoke it later.
    boost::optional<testing::task_type> saved_task;
    EXPECT_CALL(task->stream(), async_read_some(_, _))
            .Times(1)
            .WillOnce(
                WithArg<1>(
                    Invoke(
                        std::bind(
                            testing::assign,
                            std::ref(saved_task),
                            std::placeholders::_1
                        )
                    )
                )
            );

    // Extract open event.
    loop.run_one();

    // Expected that the next event will be the cancel event. We just call
    // previously saved `on_read` callback with operation aborted error to
    // imitate real stream behaviour.
    EXPECT_CALL(task->stream(), close())
            .Times(1)
            .WillOnce(
                Invoke(
                    std::bind(
                        testing::apply,
                        std::ref(saved_task),
                        std::ref(loop),
                        boost::asio::error::make_error_code(
                            boost::asio::error::operation_aborted
                        )
                    )
                )
            );
    task->cancel();

    // Extract cancel event.
    loop.run_one();

    EXPECT_EQ(1, counter);
}

TEST(urlfetch_t, Manual) {
//    urlfetch::get(url, callback, loop, timeout=default);
//    urlfetch::post(url, body, callback, loop, timeout=default);
//    urlfetch::execute(request, callback, loop);
}
