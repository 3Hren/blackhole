#include <blackhole/registry.hpp>
#include <blackhole/sink/asynchronous.hpp>

#include <src/sink/asynchronous.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// TODO: Rename directory to just "mock" to be consistent with namespace.
#include "mocks/registry.hpp"
#include "mocks/sink.hpp"

namespace blackhole {
inline namespace v1 {
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

TEST(asynchronous_t, FactoryType) {
    mock_registry_t registry;
    factory<asynchronous_t> factory(registry);

    EXPECT_EQ(std::string("asynchronous"), factory.type());
}

TEST(overflow_policy_factory_t, CreatesRegisteredPolicies) {
    EXPECT_NO_THROW(overflow_policy_factory_t().create("drop"));
    EXPECT_NO_THROW(overflow_policy_factory_t().create("wait"));
}

TEST(overflow_policy_factory_t, ThrowsIfRequestedNonRegisteredPolicy) {
    EXPECT_THROW(overflow_policy_factory_t().create(""), std::invalid_argument);
}

TEST(asynchronous_t, Builder) {
    std::unique_ptr<mock::sink_t> wrapped(new mock::sink_t);

    builder<asynchronous_t> builder(std::move(wrapped));
    auto sink = std::move(builder).build();

    EXPECT_EQ(1024, dynamic_cast<asynchronous_t&>(*sink).capacity());
}

TEST(asynchronous_t, BuilderSetFactor) {
    std::unique_ptr<mock::sink_t> wrapped(new mock::sink_t);

    builder<asynchronous_t> builder(std::move(wrapped));
    builder.factor(5);
    auto sink = std::move(builder).build();

    EXPECT_EQ(32, dynamic_cast<asynchronous_t&>(*sink).capacity());
}

TEST(asynchronous_t, BuilderSetFactorFlow) {
    std::unique_ptr<mock::sink_t> wrapped(new mock::sink_t);

    auto sink = builder<asynchronous_t>(std::move(wrapped))
        .factor(5)
        .build();

    EXPECT_EQ(32, dynamic_cast<asynchronous_t&>(*sink).capacity());
}

TEST(asynchronous_t, BuilderSetDropOverflowPolicy) {
    std::unique_ptr<mock::sink_t> wrapped(new mock::sink_t);

    builder<asynchronous_t> builder(std::move(wrapped));
    builder.drop();
    std::move(builder).build();
}

TEST(asynchronous_t, BuilderSetDropOverflowPolicyFlow) {
    std::unique_ptr<mock::sink_t> wrapped(new mock::sink_t);

    builder<asynchronous_t>(std::move(wrapped))
        .drop()
        .build();
}

TEST(asynchronous_t, BuilderSetWaitOverflowPolicy) {
    std::unique_ptr<mock::sink_t> wrapped(new mock::sink_t);

    builder<asynchronous_t> builder(std::move(wrapped));
    builder.wait();
    std::move(builder).build();
}

TEST(asynchronous_t, BuilderSetWaitOverflowPolicyFlow) {
    std::unique_ptr<mock::sink_t> wrapped(new mock::sink_t);

    builder<asynchronous_t>(std::move(wrapped))
        .wait()
        .build();
}

}  // namespace
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
