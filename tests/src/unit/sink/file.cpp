#include <system_error>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>
#include <blackhole/record.hpp>
#include <blackhole/sink/file.hpp>
#include <src/sink/file.hpp>

#include "mocks/node.hpp"
#include "mocks/registry.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {
namespace {

using ::testing::Return;
using ::testing::StrictMock;

namespace mock {

class flusher_t : public file::flusher_t {
public:
    MOCK_METHOD0(reset, void());
    MOCK_METHOD1(update, file::flusher_t::result_t(std::size_t nwritten));
};

}  // namespace mock

TEST(backend_t, Write) {
    std::unique_ptr<std::stringstream> stream(new std::stringstream);
    std::unique_ptr<mock::flusher_t> flusher(new mock::flusher_t);

    auto& stream_ = *stream;

    EXPECT_CALL(*flusher, update(11))
        .Times(1)
        .WillOnce(Return(flusher_t::result_t::idle));

    backend_t backend(std::move(stream), std::move(flusher));
    backend.write("le message");

    EXPECT_EQ("le message\n", stream_.str());
}

TEST(builder, Build) {
    builder<file_t> builder("/tmp/blackhole.log");

    builder.flush_every(100);
    std::move(builder).build();
}

TEST(builder, Chained) {
    auto sink = builder<file_t>("/tmp/blackhole.log")
        .flush_every(megabytes_t(1))
        .build();
}

TEST(factory, Type) {
    EXPECT_EQ(std::string("file"), factory<file_t>(mock_registry_t()).type());
}

TEST(factory, FromRequiresFilename) {
    StrictMock<config::testing::mock::node_t> config;

    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(nullptr));

    EXPECT_THROW(factory<file_t>(mock_registry_t()).from(config), std::invalid_argument);
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

    EXPECT_CALL(config, subscript_key("should_stat"))
        .Times(1)
        .WillOnce(Return(nullptr));

    auto sink = factory<file_t>(mock_registry_t()).from(config);
    const auto& cast = dynamic_cast<const file_t&>(*sink);

    EXPECT_EQ("/tmp/blackhole.log", cast.path());
}

TEST(factory, FlushIntervalFromConfig) {
    using config::testing::mock::node_t;

    StrictMock<node_t> config;

    auto npath = new StrictMock<node_t>;
    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(npath));

    EXPECT_CALL(*npath, to_string())
        .Times(1)
        .WillOnce(Return("/tmp/blackhole.log"));

    auto nflush = new StrictMock<node_t>;
    EXPECT_CALL(config, subscript_key("flush"))
        .Times(1)
        .WillOnce(Return(nflush));

    EXPECT_CALL(*nflush, is_uint64_())
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(*nflush, to_uint64())
        .Times(1)
        .WillOnce(Return(30));

    EXPECT_CALL(*nflush, is_string_())
        .Times(1)
        .WillOnce(Return(false));

    EXPECT_CALL(config, subscript_key("should_stat"))
        .Times(1)
        .WillOnce(Return(nullptr));

    factory<file_t>(mock_registry_t()).from(config);
}

TEST(factory, BinaryUnitFlushIntervalFromConfig) {
    using config::testing::mock::node_t;

    StrictMock<node_t> config;

    auto npath = new StrictMock<node_t>;
    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(npath));

    EXPECT_CALL(*npath, to_string())
        .Times(1)
        .WillOnce(Return("/tmp/blackhole.log"));

    auto nflush = new StrictMock<node_t>;
    EXPECT_CALL(config, subscript_key("flush"))
        .Times(1)
        .WillOnce(Return(nflush));

    EXPECT_CALL(*nflush, is_uint64_())
        .Times(1)
        .WillOnce(Return(false));

    EXPECT_CALL(*nflush, is_string_())
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(*nflush, to_string())
        .Times(1)
        .WillOnce(Return("100MB"));

    EXPECT_CALL(config, subscript_key("should_stat"))
        .Times(1)
        .WillOnce(Return(nullptr));

    factory<file_t>(mock_registry_t()).from(config);
}

}  // namespace
}  // namespace file
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
