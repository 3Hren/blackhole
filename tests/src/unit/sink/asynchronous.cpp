/// Le Plan:
/// 1. Proof of concept.
/// 2. ???
/// 3. PROFIT!

// blackhole/sink/asynchronous.hpp

#include <atomic>
#include <thread>

#include <cds/container/vyukov_mpmc_cycle_queue.h>

#include "blackhole/sink.hpp"

#include "blackhole/detail/recordbuf.hpp"

namespace blackhole {
inline namespace v1 {
namespace experimental {
namespace sink {

using detail::recordbuf_t;

/// I can imagine: drop, sleep, wait.
class overflow_policy_t {
public:
    enum class action_t {
        retry,
        drop
    };

public:
    /// Handles record queue overflow.
    ///
    /// This method is called when the queue is unable to enqueue more items. It's okay to throw
    /// exceptions from here, they will be propagated directly to the sink caller.
    virtual auto overflow() -> action_t = 0;

    virtual auto wakeup() -> void = 0;
};

class asynchronous_t : public sink_t {
    struct value_type {
        recordbuf_t record;
        std::string message;
    };

    typedef cds::container::VyukovMPSCCycleQueue<value_type> queue_type;

    queue_type queue;
    std::atomic<bool> stopped;
    std::unique_ptr<sink_t> wrapped;

    std::unique_ptr<overflow_policy_t> overflow_policy;

    std::thread thread;

public:
    /// \param factor queue capacity factor from which the queue capacity will be generated as
    ///     a value of `2 << factor`. Must fit in [0; 20] range (64 MB).
    /// \param queue_type
    /// \param overflow_policy [drop silently, drop with error, block]
    ///
    /// \throw std::invalid_argument if the factor is greater than 20.
    asynchronous_t(std::unique_ptr<sink_t> wrapped, std::size_t factor = 10);

    // asynchronous_t(std::unique_ptr<sink_t> sink,
    //                std::unique_ptr<filter_t> filter,
    //                std::unique_ptr<overflow_policy_t> overflow_policy,
    //                std::unique_ptr<exception_policy_t> exception_policy,
    //                std::size_t factor = 10);

    ~asynchronous_t();

    auto emit(const record_t& record, const string_view& message) -> void;

private:
    auto run() -> void;
};

}  // namespace sink
}  // namespace experimental
}  // namespace v1
}  // namespace blackhole

// src/sink/asynchronous.cpp

#include <unistd.h>

#include <cmath>

namespace blackhole {
inline namespace v1 {
namespace experimental {
namespace sink {
namespace {

static auto exp2(std::size_t factor) -> std::size_t {
    if (factor > 20) {
        throw std::invalid_argument("factor should fit in [0; 20] range");
    }

    return std::exp2(factor);
}

}  // namespace

class drop_overflow_policy_t : public overflow_policy_t {
    typedef overflow_policy_t::action_t action_t;

public:
    /// Drops on overlow.
    virtual auto overflow() -> action_t {
        return action_t::drop;
    }

    /// Does nothing on wakeup.
    virtual auto wakeup() -> void {}
};

class wait_overflow_policy_t : public overflow_policy_t {
    typedef overflow_policy_t::action_t action_t;

    mutable std::mutex mutex;
    std::condition_variable cv;

public:
    virtual auto overflow() -> action_t {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock);
        return action_t::retry;
    }

    virtual auto wakeup() -> void {
        cv.notify_one();
    }
};

asynchronous_t::asynchronous_t(std::unique_ptr<sink_t> wrapped, std::size_t factor) :
    queue(exp2(factor)),
    stopped(false),
    wrapped(std::move(wrapped)),
    overflow_policy(new wait_overflow_policy_t),
    thread(std::bind(&asynchronous_t::run, this))
{}

asynchronous_t::~asynchronous_t() {
    stopped.store(true);
    thread.join();
}

auto asynchronous_t::emit(const record_t& record, const string_view& message) -> void {
    if (stopped) {
        throw std::logic_error("queue is sealed");
    }

    while (true) {
        const auto enqueued = queue.enqueue_with([&](value_type& value) {
            value = {recordbuf_t(record), message.to_string()};
        });

        if (enqueued) {
            // TODO: underflow_policy->wakeup();
            return;
        } else {
            switch (overflow_policy->overflow()) {
            case overflow_policy_t::action_t::retry:
                continue;
            case overflow_policy_t::action_t::drop:
                return;
            default:
                BOOST_ASSERT(false);
            }
        }
    }
}

auto asynchronous_t::run() -> void {
    while (true) {
        value_type result;
        const auto dequeued = queue.dequeue_with([&](value_type& value) {
            result = std::move(value);
        });

        if (dequeued) {
            try {
                wrapped->emit(result.record.into_view(), result.message);
                overflow_policy->wakeup();
            } catch (...) {
                // TODO: exception_policy->process(std::current_exception()); []
            }

        } else {
            ::usleep(1000);
            // TODO: underflow_policy->underflow(); [wait for enqueue, sleep].
        }

        if (stopped && !dequeued) {
            return;
        }
    }
}

}  // namespace sink
}  // namespace experimental
}  // namespace v1
}  // namespace blackhole

// unit/sink/asynchronous.cpp

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mocks/sink.hpp"

namespace blackhole {
inline namespace v1 {
namespace experimental {
namespace sink {
namespace {

using ::testing::_;
using ::testing::Invoke;

namespace mock = testing::mock;

TEST(asynchronous_t, DelegatesEmit) {
    std::unique_ptr<mock::sink_t> wrapped(new mock::sink_t);

    EXPECT_CALL(*wrapped, emit(_, string_view("formatted message")))
        .Times(1)
        .WillOnce(Invoke([](const record_t& record, string_view) {
            ASSERT_EQ(1, record.attributes().size());
            EXPECT_EQ((attribute_list{{"key#1", {"value#1"}}}), record.attributes().at(0).get());
        }));

    asynchronous_t sink(std::move(wrapped));

    const string_view message("unformatted message");
    const attribute_list attributes{{"key#1", {"value#1"}}};
    const attribute_pack pack({attributes});
    record_t record(42, message, pack);

    sink.emit(record, "formatted message");
}

}  // namespace
}  // namespace sink
}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
