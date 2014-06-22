#include <thread>
#include <random>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include <blackhole/utils/nullptr.hpp>
#include <blackhole/platform/random.hpp>

#include "global.hpp"

struct span_t {
    const std::uint64_t trace;
    const std::uint64_t span;
    const std::uint64_t parent;

    span_t(std::uint64_t trace, std::uint64_t span, std::uint64_t parent = 0) :
        trace(trace),
        span(span),
        parent(parent)
    {}

    bool valid() const {
        return trace != 0 && span != 0;
    }

    static const span_t& invalid() {
        static const span_t span;
        return span;
    }

    bool operator==(const span_t& other) const {
        return trace == other.trace && span == other.span && parent == other.parent;
    }

    friend
    std::ostream&
    operator<<(std::ostream& stream, const span_t& span) {
        stream << "span("
               << span.trace << ", " << span.span << ", " << span.parent
               << ")";
        return stream;
    }

private:
    span_t() :
        trace(0),
        span(0),
        parent(0)
    {}
};

template<typename Value>
struct distribution_t {
    typedef Value value_type;

#if defined(HAS_CXX11_RANDOM)
private:
    std::random_device device;
    std::mt19937 generator;
    std::uniform_int_distribution<value_type> distribution;

public:
    distribution_t() :
        generator(device()),
        distribution(1, std::numeric_limits<value_type>::max())
    {}
#else
private:
    std::mt19937 generator;
    std::uniform_int<value_type> distribution;

public:
    distribution_t() :
        generator(static_cast<value_type>(std::time(nullptr))),
        distribution(1, std::numeric_limits<value_type>::max())
    {}
#endif

    value_type next() {
        return distribution(generator);
    }
};

template<typename Distribution = distribution_t<std::uint64_t>>
class random_t {
public:
    typedef Distribution distribution_type;
    typedef typename distribution_type::value_type value_type;

private:
    distribution_type distribution_;

public:
    static random_t& instance() {
        static random_t self;
        return self;
    }

    distribution_type& distribution() {
        return distribution_;
    }

    value_type next() {
        return distribution_.next();
    }

private:
    random_t() {}
};

namespace this_thread {

const span_t& current_span();

} // namespace this_thread

namespace trace {

template<class Random = random_t<>>
class context_t;

class state_t {
    boost::thread_specific_ptr<span_t> span;

    template<class Random> friend class context_t;
    friend const span_t& this_thread::current_span();

public:
    static state_t& instance() {
        static state_t self;
        return self;
    }

private:
    state_t() :
        span(&deleter)
    {}

    span_t* get() const {
        return span.get();
    }

    void reset(span_t* span = nullptr) {
        this->span.reset(span);
    }

    static void deleter(span_t*) {}
};

template<class Random>
class context_t {
public:
    typedef Random random_type;
    typedef typename random_type::value_type value_type;

private:
    span_t span;

public:
    context_t() :
        span(generate())
    {
        state_t::instance().reset(&this->span);
    }

    context_t(value_type trace) :
        span(span_t(trace, trace))
    {
        state_t::instance().reset(&this->span);
    }

    context_t(span_t span) :
        span(std::move(span))
    {
        state_t::instance().reset(&this->span);
    }

    ~context_t() {
        state_t::instance().reset();
    }

private:
    static span_t generate() {
        const value_type id = random_type::instance().next();
        if (auto current = state_t::instance().get()) {
            return span_t(current->trace, id, current->span);
        }

        return span_t(id, id);
    }
};

} // namespace guard

namespace this_thread {

const span_t& current_span() {
    auto span = trace::state_t::instance().get();
    return span ? *span : span_t::invalid();
}

} // namespace this_thread

namespace trace {

template<typename F>
class callable_t {
public:
    typedef F function_type;

private:
    const span_t span;
    function_type f;

public:
    callable_t(function_type f) :
        span(this_thread::current_span()),
        f(std::move(f))
    {}

    template<typename... Args>
    void operator()(Args&&... args) {
        trace::context_t<> context(span);
        f(std::forward<Args>(args)...);
    }
};

template<typename F>
callable_t<F> wrap(F&& f) {
    return callable_t<F>(std::forward<F>(f));
}

} // namespace trace

namespace mock {

class distribution_t {
public:
    typedef std::uint64_t value_type;

    MOCK_METHOD0(next, value_type());
};

} // namespace mock

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

        EXPECT_EQ(0, this_thread::current_span().trace);
        EXPECT_EQ(0, this_thread::current_span().span);
        EXPECT_EQ(0, this_thread::current_span().parent);
    }

    EXPECT_EQ(0, this_thread::current_span().trace);
    EXPECT_EQ(0, this_thread::current_span().span);
    EXPECT_EQ(0, this_thread::current_span().parent);
}

#include <boost/asio.hpp>

void checker(int& counter, span_t expected) {
    EXPECT_EQ(expected, this_thread::current_span());
    counter++;
}

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
            std::bind(&checker, std::ref(counter), span_t(43, 43))
        )
    );

    loop.run();
    EXPECT_EQ(1, counter);
}

void create_trace(boost::asio::io_service& loop,
                  int& counter,
                  std::uint64_t id)
{
    trace::context_t<random_t<mock::distribution_t>> context(id);
    loop.post(
        trace::wrap(
            std::bind(&checker, std::ref(counter), span_t(id, id))
        )
    );
}

TEST(Context, SimultaneousSimpleTraceHandlingViaEventLoop) {
    boost::asio::io_service loop;
    int counter = 0;

    loop.post(std::bind(&create_trace, std::ref(loop), std::ref(counter), 1));
    loop.post(std::bind(&create_trace, std::ref(loop), std::ref(counter), 2));

    loop.run();
    EXPECT_EQ(2, counter);
}

//!@todo: Letter to myself in the future.
//! After two week vacation you will remember nothing about this subproject.
//! 1. More tests:
//! 1.2. Nested contexts: 2 traces, 2 spans in each.
//! 1.3. Two threads.
//! 1.4. Nested contexts in threads.
//! 1.5. Event loop in threads.
//! 1.6. Single event loop in multiple threads.
//! 1.7. Less code magic with tests. Less code templatization. More concrete
//!      hardcoded cases. In simplifies further reading.
//! 2. Think about returning span context after `trace::context_t` leaves its
//!    scope.

namespace random_delay {

void check_first(int& counter, const boost::system::error_code&) {
    EXPECT_EQ(span_t(44, 44, 0), this_thread::current_span());
    EXPECT_EQ(0, counter);
    counter++;
}

void check_second(int& counter) {
    EXPECT_EQ(span_t(45, 45, 0), this_thread::current_span());
    EXPECT_EQ(1, counter);
    counter++;
}

// Emulate socket accept handler.
void create_first_trace(boost::asio::io_service& loop, int& counter) {
    trace::context_t<random_t<mock::distribution_t>> context(44);
    boost::asio::deadline_timer timer(loop);
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

TEST(Context, AsynchronousContextWithRandomDelay) {
    //! Sequence diagram:
    //! --44----c1
    //! ----45c2--

    boost::asio::io_service loop;
    int counter = 0;

    loop.post(
        std::bind(
            &random_delay::create_first_trace,
            std::ref(loop),
            std::ref(counter)
        )
    );
    loop.post(
        std::bind(
            &random_delay::create_second_trace,
            std::ref(loop),
            std::ref(counter)
        )
    );

    loop.run();

    EXPECT_EQ(span_t::invalid(), this_thread::current_span());

    EXPECT_EQ(2, counter);
}
void print(std::initializer_list<std::string> list) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    auto span = this_thread::current_span();
    std::cout << "[" << std::setw(20) << span.trace << "] ["
              << std::setw(20) << span.span << "]: ";
    for (auto it = list.begin(); it != list.end(); ++it) {
        std::cout << *it;
    }
    std::cout << std::endl;
}

void func(int i) {
    print({ "fn", boost::lexical_cast<std::string>(i), " start " });
    {
        trace::context_t<> g;
        print({ "fn", boost::lexical_cast<std::string>(i), " mid " });
        if (i <= 1) {
            func(i + 1);
        }
    }

    print({ "fn", boost::lexical_cast<std::string>(i), " end " });
}

TEST(Manual, 1) {
    print({ "main begin" });
    {
        trace::context_t<> g;
        print({ "before invoke" });
        func(1);
        print({ "after invoke" });
    }
    print({ "main end" });
}

TEST(Manual, 2) {
    print({ "main begin" });
    {
        trace::context_t<> g;
        print({ "before invoke" });
        std::thread t1(trace::wrap(std::bind(&func, 1)));
        std::thread t2(trace::wrap(std::bind(&func, 1)));
        print({ "before join" });
        t1.join();
        print({ "middle join" });
        t2.join();
        print({ "after join" });
    }
    print({ "main end" });
}

#include <boost/asio.hpp>

void on_timer(int i, const boost::system::error_code&) {
    print({"timer", boost::lexical_cast<std::string>(i)});
    func(i);
}

TEST(Manual, 3) {
    trace::context_t<> g;
    print({ "main begin" });
    boost::asio::io_service loop;
    boost::asio::deadline_timer timer1(loop);
    boost::asio::deadline_timer timer2(loop);

    std::function<void(const boost::system::error_code&)> fn = std::bind(&on_timer, 1, std::placeholders::_1);

    timer1.expires_from_now(boost::posix_time::milliseconds(100));
    timer2.expires_from_now(boost::posix_time::milliseconds(100));

    timer1.async_wait(trace::wrap(std::bind(&on_timer, 1, std::placeholders::_1)));
    timer2.async_wait(trace::wrap(std::bind(&on_timer, 1, std::placeholders::_1)));
    loop.run();
    print({ "main end" });
}

TEST(Manual, 4) {
//    run loop in the separate thread
//    run service with 2 timers
//    run service with 2 timers
//    join
}
