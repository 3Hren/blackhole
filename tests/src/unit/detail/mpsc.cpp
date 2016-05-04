#include <gtest/gtest.h>

#include <cds/container/vyukov_mpmc_cycle_queue.h>

namespace blackhole {
inline namespace v1 {
namespace {

TEST(mpsc_queue_t, EnqueueDequeue) {
    cds::container::VyukovMPSCCycleQueue<int> queue(1024);

    EXPECT_EQ(0, queue.size());

    queue.enqueue_with([&](int &cell) {
        cell = 100500;
    });

    // We didn't enable `empty_item_counter` feature, so no counting occurs.
    EXPECT_EQ(0, queue.size());

    int result = 0;
    const auto dequeued = queue.dequeue(result);

    ASSERT_TRUE(dequeued);
    EXPECT_EQ(100500, result);
    EXPECT_EQ(0, queue.size());
}

}  // namespace
}  // namespace v1
}  // namespace blackhole
