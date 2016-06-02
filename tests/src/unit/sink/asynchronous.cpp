#include <blackhole/registry.hpp>
#include <blackhole/sink/asynchronous.hpp>
#include <blackhole/detail/sink/asynchronous.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// TODO: Rename directory to just "mock" to be consistent with namespace.
#include "mocks/sink.hpp"
#include "mocks/registry.hpp"

namespace blackhole {
inline namespace v1 {
namespace experimental {
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

}  // namespace
}  // namespace sink
}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
