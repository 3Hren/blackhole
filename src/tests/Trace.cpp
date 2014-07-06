#include <thread>

#include <boost/asio.hpp>

#include <blackhole/trace.hpp>
#include <blackhole/utils/atomic.hpp>

#include "global.hpp"
#include "mocks/trace.hpp"

TEST(Context, Initiating) {
    auto& distribution = random_t<mock::distribution_t>::instance().distribution();
    EXPECT_CALL(distribution, next())
            .Times(1)
            .WillOnce(Return(42));

    EXPECT_EQ(0, this_thread::current_span().trace);
    EXPECT_EQ(0, this_thread::current_span().span);
    EXPECT_EQ(0, this_thread::current_span().parent);

    trace::context_t<random_t<mock::distribution_t>> context;
    EXPECT_EQ(42, this_thread::current_span().trace);
    EXPECT_EQ(42, this_thread::current_span().span);
    EXPECT_EQ(0, this_thread::current_span().parent);
}

TEST(Context, ResetContextAfterLeavingScope) {
    auto& distribution = random_t<mock::distribution_t>::instance().distribution();
    EXPECT_CALL(distribution, next())
            .Times(1)
            .WillOnce(Return(42));

    EXPECT_EQ(0, this_thread::current_span().trace);
    EXPECT_EQ(0, this_thread::current_span().span);
    EXPECT_EQ(0, this_thread::current_span().parent);

    {
        trace::context_t<random_t<mock::distribution_t>> context;
        EXPECT_EQ(42, this_thread::current_span().trace);
        EXPECT_EQ(42, this_thread::current_span().span);
        EXPECT_EQ(0, this_thread::current_span().parent);
    }

    // Trace context is completery reset.
    EXPECT_EQ(0, this_thread::current_span().trace);
    EXPECT_EQ(0, this_thread::current_span().span);
    EXPECT_EQ(0, this_thread::current_span().parent);
}

TEST(Context, ExactlyResetNotFallbackContextAfterLeavingScope) {
    auto& distribution = random_t<mock::distribution_t>::instance().distribution();
    EXPECT_CALL(distribution, next())
            .Times(2)
            .WillOnce(Return(42))
            .WillOnce(Return(100500));

    EXPECT_EQ(0, this_thread::current_span().trace);
    EXPECT_EQ(0, this_thread::current_span().span);
    EXPECT_EQ(0, this_thread::current_span().parent);

    {
        trace::context_t<random_t<mock::distribution_t>> context;
        EXPECT_EQ(42, this_thread::current_span().trace);
        EXPECT_EQ(42, this_thread::current_span().span);
        EXPECT_EQ(0, this_thread::current_span().parent);

        {
            trace::context_t<random_t<mock::distribution_t>> nested;
            EXPECT_EQ(42, this_thread::current_span().trace);
            EXPECT_EQ(100500, this_thread::current_span().span);
            EXPECT_EQ(42, this_thread::current_span().parent);
        }

        // Trace context is partially reset at this moment.
        EXPECT_EQ(42, this_thread::current_span().trace);
        EXPECT_EQ(42, this_thread::current_span().span);
        EXPECT_EQ(0, this_thread::current_span().parent);
    }

    // Trace context is completery reset now.
    EXPECT_EQ(0, this_thread::current_span().trace);
    EXPECT_EQ(0, this_thread::current_span().span);
    EXPECT_EQ(0, this_thread::current_span().parent);
}

namespace testing {

namespace loop_handling {

void check(int& counter) {
    EXPECT_EQ(span_t(43, 43), this_thread::current_span());
    counter++;
}

} // namespace loop_handling

} // namespace testing

TEST(Context, AsynchronousHandlingViaEventLoop) {
    boost::asio::io_service loop;
    int counter = 0;

    auto& distribution = random_t<mock::distribution_t>::instance().distribution();
    EXPECT_CALL(distribution, next())
            .Times(1)
            .WillOnce(Return(43));
    trace::context_t<random_t<mock::distribution_t>> context;

    loop.post(
        trace::wrap(
            std::bind(&loop_handling::check, std::ref(counter))
        )
    );

    loop.run();
    EXPECT_EQ(1, counter);
}

namespace testing {

namespace loop_handling {

namespace simultaneous {

void check_first(int& counter) {
    EXPECT_EQ(span_t(1, 1), this_thread::current_span());
    EXPECT_EQ(0, counter);
    counter++;
}

void check_second(int& counter) {
    EXPECT_EQ(span_t(2, 2), this_thread::current_span());
    EXPECT_EQ(1, counter);
    counter++;
}

void create_first_trace(boost::asio::io_service& loop, int& counter) {
    trace::context_t<random_t<mock::distribution_t>> context(1);
    loop.post(trace::wrap(std::bind(&check_first, std::ref(counter))));
}

void create_second_trace(boost::asio::io_service& loop, int& counter) {
    trace::context_t<random_t<mock::distribution_t>> context(2);
    loop.post(trace::wrap(std::bind(&check_second, std::ref(counter))));
}

} // namespace simultaneous

} // namespace loop_handling

} // namespace testing

TEST(Context, SimultaneousSimpleTraceHandlingViaEventLoop) {
    boost::asio::io_service loop;
    int counter = 0;

    loop.post(
        std::bind(
            &loop_handling::simultaneous::create_first_trace,
            std::ref(loop),
            std::ref(counter)
        )
    );
    loop.post(
        std::bind(
            &loop_handling::simultaneous::create_second_trace,
            std::ref(loop),
            std::ref(counter)
        )
    );

    loop.run();
    EXPECT_EQ(2, counter);
}

namespace testing {

namespace loop_handling {

namespace random_delay {

void check_first(int& counter, const boost::system::error_code& ec) {
    ASSERT_EQ(0, ec.value());
    EXPECT_EQ(span_t(44, 44, 0), this_thread::current_span());
    EXPECT_EQ(1, counter);
    counter++;
}

void check_second(int& counter) {
    EXPECT_EQ(span_t(45, 45, 0), this_thread::current_span());
    EXPECT_EQ(0, counter);
    counter++;
}

// Emulate socket accept handler.
void create_first_trace(boost::asio::deadline_timer& timer, int& counter) {
    trace::context_t<random_t<mock::distribution_t>> context(44);
    timer.expires_from_now(boost::posix_time::milliseconds(1));
    timer.async_wait(
        trace::wrap(
            std::bind(&check_first, std::ref(counter), std::placeholders::_1)
        )
    );
}

void create_second_trace(boost::asio::io_service& loop, int& counter) {
    trace::context_t<random_t<mock::distribution_t>> context(45);
    loop.post(
        trace::wrap(
            std::bind(&check_second, std::ref(counter))
        )
    );
}

} // namespace random_delay

} // namespace loop_handling

} // namespace testing

TEST(Context, AsynchronousContextWithRandomDelay) {
    //! Sequence diagram:
    //! --44----c1
    //! ----45c2--

    boost::asio::io_service loop;
    int counter = 0;

    boost::asio::deadline_timer timer(loop);
    loop.post(
        std::bind(
            &loop_handling::random_delay::create_first_trace,
            std::ref(timer),
            std::ref(counter)
        )
    );

    loop.post(
        std::bind(
            &loop_handling::random_delay::create_second_trace,
            std::ref(loop),
            std::ref(counter)
        )
    );

    loop.run_one();
    loop.run_one();

    EXPECT_EQ(0, counter);

    loop.run_one();
    EXPECT_EQ(1, counter);

    loop.run_one();
    EXPECT_EQ(2, counter);

    EXPECT_EQ(span_t::invalid(), this_thread::current_span());
}

namespace testing {

namespace thread_handling {

void check(std::atomic<int>& counter) {
    EXPECT_EQ(span_t(46, 46, 0), this_thread::current_span());
    counter++;
}

} // namespace thread_handling

} // namespace testing

TEST(Context, AsynchronousHandlingViaThreads) {
    std::atomic<int> counter(0);

    auto& distribution = random_t<mock::distribution_t>::instance().distribution();
    EXPECT_CALL(distribution, next())
            .Times(1)
            .WillOnce(Return(46));
    trace::context_t<random_t<mock::distribution_t>> context;

    std::thread thread(
        trace::wrap(std::bind(&thread_handling::check, std::ref(counter)))
    );
    thread.join();

    EXPECT_EQ(1, counter);
}
