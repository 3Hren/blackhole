/// Le Plan:
/// 1. Proof of concept.
/// 2. ???
/// 3. PROFIT!

// blackhole/sink/asynchronous.hpp

#include <atomic>
#include <thread>

#include <cds/container/vyukov_mpmc_cycle_queue.h>

#include "blackhole/sink.hpp"

#include "blackhole/detail/record.owned.hpp"

namespace blackhole {
inline namespace v1 {
namespace experimental {
namespace sink {

class asynchronous_t : public sink_t {
    struct value_type {
        detail::owned<record_t> record;
        std::string message;
    };

    // union union_type {
    //     struct {} uninitialized;
    //     value_type value;
    //
    //     constexpr union_type() noexcept :
    //         uninitialized({})
    //     {}
    //
    //     union_type(const union_type& other) = delete;
    //
    //     union_type(union_type&& other) noexcept :
    //         value(std::move(other.value))
    //     {}
    //
    //     ~union_type() {}
    //
    //     auto operator=(const union_type& other) -> union_type& = delete;
    //
    //     auto operator=(union_type&& other) noexcept -> union_type& {
    //         value = std::move(other.value);
    //         return *this;
    //     }
    // };

    typedef cds::container::VyukovMPSCCycleQueue<value_type> queue_type;

    queue_type queue;
    std::atomic<bool> running;
    std::unique_ptr<sink_t> wrapped;

    std::thread thread;

public:
    /// \param factor queue capacity factor from which the queue capacity will be generated as
    ///     a value of `2 << factor`. Must fit in [0; 20] range (64 MB).
    /// \param queue_type
    /// \param overflow_policy [drop silently, drop with error, block]
    asynchronous_t(std::unique_ptr<sink_t> wrapped, std::size_t factor = 10);

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
#include <iostream>

namespace blackhole {
inline namespace v1 {
namespace experimental {
namespace sink {

asynchronous_t::asynchronous_t(std::unique_ptr<sink_t> wrapped, std::size_t factor) :
    queue(2 << factor),
    running(true),
    wrapped(std::move(wrapped)),
    thread(std::bind(&asynchronous_t::run, this))
{}

asynchronous_t::~asynchronous_t() {
    running = false;
    thread.join();
}

auto asynchronous_t::emit(const record_t& record, const string_view& message) -> void {
    if (!running) {
        return;
    }

    const auto enqueued = queue.enqueue_with([&](value_type& value) {
        value = {detail::owned<record_t>(record), message.to_string()};
    });

    // TODO: If failed, some kind of overflow policy is immediately involved.
}

auto asynchronous_t::run() -> void {
    while (true) {
        value_type result;
        const auto dequeued = queue.dequeue_with([&](value_type& value) {
            result = std::move(value);
        });

        std::cout << std::boolalpha << dequeued << ":" << result.message << std::endl;
        if (dequeued) {
            // TODO: What to do with exceptions?
            wrapped->emit(result.record.into_view(), result.message);
        } else {
            // Sleep or CV.
            ::usleep(1000);
        }

        if (!running && !dequeued) {
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

namespace mock = testing::mock;

TEST(asynchronous_t, DelegatesEmit) {
    std::unique_ptr<mock::sink_t> wrapped(new mock::sink_t);

    EXPECT_CALL(*wrapped, emit(_, string_view("formatted message")))
        .Times(1);

    asynchronous_t sink(std::move(wrapped));

    const string_view message("");
    const attribute_pack pack;
    record_t record(42, message, pack);

    sink.emit(record, "formatted message");
}

}  // namespace
}  // namespace sink
}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
