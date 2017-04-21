#include <atomic>
#include <thread>

#include <cds/container/vyukov_mpmc_cycle_queue.h>

#include "blackhole/sink.hpp"

#include "../recordbuf.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

/// I can imagine: drop, sleep, wait.
class overflow_policy_t {
public:
    enum class action_t {
        retry,
        drop
    };

public:
    virtual ~overflow_policy_t() {}

    /// Handles record queue overflow.
    ///
    /// This method is called when the queue is unable to enqueue more items. It's okay to throw
    /// exceptions from here, they will be propagated directly to the sink caller.
    virtual auto overflow() -> action_t = 0;

    virtual auto wakeup() -> void = 0;
};

class overflow_policy_factory_t {
public:
    auto create(const std::string& name) const -> std::unique_ptr<overflow_policy_t>;
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
    asynchronous_t(std::unique_ptr<sink_t> wrapped, std::size_t factor = 10);

    // TODO: Full customization.
    asynchronous_t(std::unique_ptr<sink_t> sink,
                   std::size_t factor,
                //    std::unique_ptr<filter_t> filter,
                //    std::unique_ptr<underflow_policy_t> underflow_policy,
                //    std::unique_ptr<exception_policy_t> exception_policy,
                   std::unique_ptr<overflow_policy_t> overflow_policy);

    ~asynchronous_t();

    auto emit(const record_t& record, const string_view& message) -> void;

private:
    auto run() -> void;
};

}  // namespace sink
}  // namespace v1
}  // namespace blackhole
