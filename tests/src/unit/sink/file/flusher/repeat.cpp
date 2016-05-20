#include <gtest/gtest.h>

#include <blackhole/detail/sink/file/flusher/repeat.hpp>

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {
namespace flusher {
namespace {

TEST(repeat_t, Default) {
    repeat_t flusher(3);

    EXPECT_EQ(3, flusher.threshold());
    EXPECT_EQ(0, flusher.count());
}

TEST(repeat_t, Zero) {
    repeat_t flusher(0);

    EXPECT_EQ(std::numeric_limits<std::size_t>::max(), flusher.threshold());
}

TEST(repeat_t, Update) {
    repeat_t flusher(3);

    EXPECT_EQ(flusher_t::idle, flusher.update(1));
    EXPECT_EQ(flusher_t::idle, flusher.update(1));
    EXPECT_EQ(flusher_t::flush, flusher.update(1));
    EXPECT_EQ(flusher_t::idle, flusher.update(1));
    EXPECT_EQ(flusher_t::idle, flusher.update(1));
    EXPECT_EQ(flusher_t::flush, flusher.update(1));
}

TEST(repeat_t, CounterOverflow) {
    repeat_t flusher(3);

    flusher.update(1);
    EXPECT_EQ(1, flusher.count());

    flusher.update(42);
    EXPECT_EQ(2, flusher.count());

    flusher.update(1);
    EXPECT_EQ(0, flusher.count());
}

TEST(repeat_t, Reset) {
    repeat_t flusher(3);

    flusher.update(1);
    flusher.update(1);
    EXPECT_EQ(2, flusher.count());

    flusher.reset();
    EXPECT_EQ(0, flusher.count());
}

TEST(repeat_t, ZeroWrite) {
    repeat_t flusher(3);

    flusher.update(0);
    EXPECT_EQ(0, flusher.count());
}

TEST(repeat_factory_t, Init) {
    EXPECT_EQ(42, repeat_factory_t(42).threshold());
}

TEST(repeat_factory_t, CreatesRepeatFlusher) {
    auto factory = repeat_factory_t(42);
    auto flusher = factory.create();
    auto repeat = dynamic_cast<const repeat_t&>(*flusher);

    EXPECT_EQ(42, repeat.threshold());
}

}  // namespace
}  // flusher
}  // namespace file
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
