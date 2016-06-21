#include <syslog.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/stdext/string_view.hpp>
#include <blackhole/sink/syslog.hpp>

#include <blackhole/detail/procname.hpp>
#include <blackhole/detail/sink/syslog.hpp>

#include "mocks/node.hpp"
#include "mocks/registry.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace {

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::StrictMock;

TEST(syslog_t, Type) {
    EXPECT_EQ(std::string("syslog"), factory<sink::syslog_t>(mock_registry_t()).type());
}

TEST(syslog_t, Factory) {
    using config::testing::mock::node_t;

    StrictMock<node_t> config;

    auto priorities_node = new node_t;
    EXPECT_CALL(config, subscript_key("priorities"))
        .Times(1)
        .WillOnce(Return(priorities_node));

    node_t priority_item_node;
    EXPECT_CALL(*priorities_node, each(_))
        .Times(1)
        .WillOnce(Invoke([&](const node_t::each_function& fn) {
            for (int i = 0; i < 4; ++i) {
                fn(priority_item_node);
            }
        }));

    EXPECT_CALL(priority_item_node, to_sint64())
        .Times(4)
        .WillOnce(Return(0))
        .WillOnce(Return(2))
        .WillOnce(Return(5))
        .WillOnce(Return(9));

    auto sink = factory<syslog_t>(mock_registry_t()).from(config);
    const auto& cast = dynamic_cast<const syslog_t&>(*sink);

    EXPECT_EQ((std::vector<int>{0, 2, 5, 9}), cast.priorities());
}

TEST(syslog_t, Default) {
    syslog_t syslog;

    EXPECT_EQ(detail::procname().to_string(), syslog.identity());
    EXPECT_EQ(LOG_PID, syslog.option());
    EXPECT_EQ(LOG_USER, syslog.facility());
}

}  // namespace
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
