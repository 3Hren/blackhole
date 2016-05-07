#include "blackhole/sink/asynchronous.hpp"

#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/registry.hpp"

#include "blackhole/detail/sink/asynchronous.hpp"

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

    return static_cast<std::size_t>(std::exp2(factor));
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

auto factory<sink::asynchronous_t>::type() const noexcept -> const char* {
    return "asynchronous";
}

auto factory<sink::asynchronous_t>::from(const config::node_t& config) const -> std::unique_ptr<sink_t> {
    auto factory = registry.sink(*config["sink"]["type"].to_string());
    return std::unique_ptr<sink_t>(new sink::asynchronous_t(factory(*config["sink"].unwrap())));
}

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
