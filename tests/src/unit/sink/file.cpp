#include <system_error>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>
#include <blackhole/record.hpp>
#include <blackhole/sink/file.hpp>
#include <blackhole/detail/sink/file.hpp>

#include "mocks/node.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {
namespace {

using experimental::factory;

using ::testing::Return;
using ::testing::StrictMock;

TEST(repeat_flusher_t, Default) {
    repeat_flusher_t flusher(3);

    EXPECT_EQ(3, flusher.threshold());
    EXPECT_EQ(0, flusher.count());
}

TEST(repeat_flusher_t, Update) {
    repeat_flusher_t flusher(3);

    EXPECT_EQ(flusher_t::idle, flusher.update(1));
    EXPECT_EQ(flusher_t::idle, flusher.update(1));
    EXPECT_EQ(flusher_t::flush, flusher.update(1));
    EXPECT_EQ(flusher_t::idle, flusher.update(1));
    EXPECT_EQ(flusher_t::idle, flusher.update(1));
    EXPECT_EQ(flusher_t::flush, flusher.update(1));
}

TEST(repeat_flusher_t, CounterOverflow) {
    repeat_flusher_t flusher(3);

    flusher.update(1);
    EXPECT_EQ(1, flusher.count());

    flusher.update(42);
    EXPECT_EQ(2, flusher.count());

    flusher.update(1);
    EXPECT_EQ(0, flusher.count());
}

TEST(repeat_flusher_t, Reset) {
    repeat_flusher_t flusher(3);

    flusher.update(1);
    flusher.update(1);
    EXPECT_EQ(2, flusher.count());

    flusher.reset();
    EXPECT_EQ(0, flusher.count());
}

TEST(repeat_flusher_t, ZeroWrite) {
    repeat_flusher_t flusher(3);

    flusher.update(0);
    EXPECT_EQ(0, flusher.count());
}

TEST(bytecount_flusher_t, Default) {
    bytecount_flusher_t flusher(1024);

    EXPECT_EQ(1024, flusher.threshold());
    EXPECT_EQ(0, flusher.count());
}

TEST(bytecount_flusher_t, Update) {
    bytecount_flusher_t flusher(1024);

    EXPECT_EQ(flusher_t::idle, flusher.update(10));
    EXPECT_EQ(flusher_t::idle, flusher.update(1000));
    EXPECT_EQ(flusher_t::flush, flusher.update(14));
    EXPECT_EQ(flusher_t::idle, flusher.update(1023));
    EXPECT_EQ(flusher_t::flush, flusher.update(1));
    EXPECT_EQ(flusher_t::flush, flusher.update(100500)); // 100500 % 1024 = 148
    EXPECT_EQ(flusher_t::idle, flusher.update(2));
    EXPECT_EQ(flusher_t::flush, flusher.update(875));
}

TEST(bytecount_flusher_t, CounterOverflow) {
    bytecount_flusher_t flusher(1024);

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

TEST(bytecount_flusher_t, Reset) {
    bytecount_flusher_t flusher(1024);

    flusher.update(10);
    EXPECT_EQ(10, flusher.count());

    flusher.update(1000);
    EXPECT_EQ(1010, flusher.count());

    flusher.reset();
    EXPECT_EQ(0, flusher.count());
}

TEST(repeat_flusher_factory_t, Init) {
    EXPECT_EQ(42, repeat_flusher_factory_t(42).threshold());
}

TEST(repeat_flusher_factory_t, CreatesRepeatFlusher) {
    auto factory = repeat_flusher_factory_t(42);
    auto flusher = factory.create();
    auto repeat = dynamic_cast<const repeat_flusher_t&>(*flusher);

    EXPECT_EQ(42, repeat.threshold());
}

TEST(bytecount_flusher_factory_t, Init) {
    EXPECT_EQ(1024, bytecount_flusher_factory_t(1024).threshold());
}

TEST(bytecount_flusher_factory_t, CreatesByteCountFlusher) {
    auto factory = bytecount_flusher_factory_t(1024);
    auto flusher = factory.create();
    auto repeat = dynamic_cast<const bytecount_flusher_t&>(*flusher);

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

TEST(backend_t, ThrowsIfUnableToOpenStream) {
    EXPECT_THROW(backend_t("/__mythic/file.log", 1), std::system_error);
}

TEST(file_t, IntervalSanitizer) {
    file_t sink("", 0);

    EXPECT_NE(0, sink.interval());
    EXPECT_EQ(std::numeric_limits<std::size_t>::max(), sink.interval());
}

TEST(file_t, IntervalOverflow) {
    std::size_t interval = 3;
    std::size_t counter = 0;

    counter = (counter + 1) % interval;
    EXPECT_EQ(1, counter);

    counter = (counter + 1) % interval;
    EXPECT_EQ(2, counter);

    counter = (counter + 1) % interval;
    EXPECT_EQ(0, counter);
}

TEST(factory, Type) {
    EXPECT_EQ(std::string("file"), factory<file_t>().type());
}

TEST(factory, FromRequiresFilename) {
    StrictMock<config::testing::mock::node_t> config;

    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(nullptr));

    EXPECT_THROW(factory<file_t>().from(config), std::invalid_argument);
}

TEST(factory, PatternFromConfig) {
    using config::testing::mock::node_t;

    StrictMock<node_t> config;

    auto npath = new node_t;

    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(npath));

    EXPECT_CALL(*npath, to_string())
        .Times(1)
        .WillOnce(Return("/tmp/blackhole.log"));

    EXPECT_CALL(config, subscript_key("flush"))
        .Times(1)
        .WillOnce(Return(nullptr));

    auto sink = factory<file_t>().from(config);
    const auto& cast = dynamic_cast<const file_t&>(*sink);

    EXPECT_EQ("/tmp/blackhole.log", cast.path());
}

TEST(factory, FlushIntervalFromConfig) {
    using config::testing::mock::node_t;

    StrictMock<node_t> config;

    auto npath = new node_t;
    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(npath));

    EXPECT_CALL(*npath, to_string())
        .Times(1)
        .WillOnce(Return("/tmp/blackhole.log"));

    auto nflush = new node_t;
    EXPECT_CALL(config, subscript_key("flush"))
        .Times(1)
        .WillOnce(Return(nflush));

    EXPECT_CALL(*nflush, to_uint64())
        .Times(1)
        .WillOnce(Return(30));

    factory<file_t>().from(config);
}

}  // namespace
}  // namespace file
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
