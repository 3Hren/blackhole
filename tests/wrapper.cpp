#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/thread/tss.hpp>

#include <blackhole/logger.hpp>
#include <blackhole/scoped/keeper.hpp>
#include <blackhole/wrapper.hpp>

#include <blackhole/extensions/facade.hpp>
#include <blackhole/handler.hpp>
#include <blackhole/root.hpp>

#include "mocks/logger.hpp"

namespace blackhole {
namespace testing {

using ::testing::Return;
using ::testing::_;

TEST(Wrapper, Constructor) {
    mock::logger_t logger;

    wrapper_t wrapper(logger, {
        {"key#0", {0}},
        {"key#1", {"value#1"}}
    });

    const view_of<attributes_t>::type expected = {
        {"key#0", {0}},
        {"key#1", {"value#1"}}
    };

    EXPECT_EQ(expected, wrapper.attributes());
}

TEST(wrapper_t, DelegatesScoped) {
    // NOTE: This hack is required to test scoped attributes mechanics.
    boost::thread_specific_ptr<scoped_t> context([](scoped_t*) {});

    mock::logger_t logger;
    wrapper_t wrapper(logger, {});

    // TODO: Implement operator== for attribute. Refactor all tests to check.
    EXPECT_CALL(logger, context())
        .Times(1)
        .WillOnce(Return(&context));

    const scoped::keeper_t scoped(wrapper, {});
}

}  // namespace testing
}  // namespace blackhole
