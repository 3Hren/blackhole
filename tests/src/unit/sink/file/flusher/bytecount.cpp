#include <gtest/gtest.h>

#include <blackhole/detail/sink/file/flusher/bytecount.hpp>

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {
namespace flusher {
namespace {

TEST(bytecount_t, Default) {
    bytecount_t flusher(1024);

    EXPECT_EQ(1024, flusher.threshold());
    EXPECT_EQ(0, flusher.count());
}

TEST(bytecount_t, Update) {
    bytecount_t flusher(1024);

    EXPECT_EQ(flusher_t::idle, flusher.update(10));
    EXPECT_EQ(flusher_t::idle, flusher.update(1000));
    EXPECT_EQ(flusher_t::flush, flusher.update(14));
    EXPECT_EQ(flusher_t::idle, flusher.update(1023));
    EXPECT_EQ(flusher_t::flush, flusher.update(1));
    EXPECT_EQ(flusher_t::flush, flusher.update(100500)); // 100500 % 1024 = 148
    EXPECT_EQ(flusher_t::idle, flusher.update(2));
    EXPECT_EQ(flusher_t::flush, flusher.update(875));
}

TEST(bytecount_t, CounterOverflow) {
    bytecount_t flusher(1024);

    flusher.update(10);
    EXPECT_EQ(10, flusher.count());

    flusher.update(1000);
    EXPECT_EQ(1010, flusher.count());

    flusher.update(14);
    EXPECT_EQ(0, flusher.count());

    flusher.update(1023);
    EXPECT_EQ(1023, flusher.count());

    flusher.update(1);
    EXPECT_EQ(0, flusher.count());

    flusher.update(100500);
    EXPECT_EQ(148, flusher.count());

    flusher.update(2);
    EXPECT_EQ(150, flusher.count());

    flusher.update(875);
    EXPECT_EQ(1, flusher.count());
}

TEST(bytecount_t, Reset) {
    bytecount_t flusher(1024);

    flusher.update(10);
    EXPECT_EQ(10, flusher.count());

    flusher.update(1000);
    EXPECT_EQ(1010, flusher.count());

    flusher.reset();
    EXPECT_EQ(0, flusher.count());
}

TEST(bytecount_factory_t, Init) {
    EXPECT_EQ(1024, bytecount_factory_t(1024).threshold());
}

TEST(bytecount_factory_t, CreatesByteCountFlusher) {
    auto factory = bytecount_factory_t(1024);
    auto flusher = factory.create();
    auto repeat = dynamic_cast<const bytecount_t&>(*flusher);

    EXPECT_EQ(1024, repeat.threshold());
}

TEST(parse_dunit, WithoutUnit) {
    EXPECT_EQ(1024, parse_dunit("1024"));
}

TEST(parse_dunit, KnownUnits) {
    EXPECT_EQ(1024, parse_dunit("1024B"));
    EXPECT_EQ(1024 * 1e3, parse_dunit("1024kB"));
    EXPECT_EQ(1024 * 1e6, parse_dunit("1024MB"));
    EXPECT_EQ(1024 * 1e9, parse_dunit("1024GB"));

    EXPECT_EQ(1024 * 1ULL << 10, parse_dunit("1024KiB"));
    EXPECT_EQ(1024 * 1ULL << 20, parse_dunit("1024MiB"));
    EXPECT_EQ(1024 * 1ULL << 30, parse_dunit("1024GiB"));
}

TEST(parse_dunit, ThrowsOnUnknownUnit) {
    EXPECT_THROW(parse_dunit("1024Hz"), std::invalid_argument);
}

}  // namespace
}  // namespace flusher
}  // namespace file
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
