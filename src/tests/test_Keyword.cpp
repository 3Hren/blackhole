#include <blackhole/keyword/process.hpp>
#include <blackhole/keyword/timestamp.hpp>
#include <blackhole/keyword/thread.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(timestamp, Init) {
    // Initializer function should return current timestamp.
    timeval min, max;
    gettimeofday(&min, nullptr);

    auto value = keyword::init::timestamp();

    gettimeofday(&max, nullptr);

    EXPECT_TRUE(1000000 * min.tv_sec + min.tv_usec <= 1000000 * value.tv_sec + value.tv_usec);
    EXPECT_TRUE(1000000 * value.tv_sec + value.tv_usec <= 1000000 * max.tv_sec + max.tv_usec);
}

#ifdef BLACKHOLE_HAS_ATTRIBUTE_PID
TEST(pid, Init) {
    EXPECT_EQ(::getpid(), keyword::init::pid());
}
#endif

#ifdef BLACKHOLE_HAS_ATTRIBUTE_TID
TEST(tid, Init) {
    EXPECT_EQ(this_thread::get_id<std::string>(), keyword::init::tid());
}
#endif
