#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/thread/tss.hpp>

#include <blackhole/logger.hpp>
#include <blackhole/scope/holder.hpp>
#include <blackhole/scope/manager.hpp>
#include <blackhole/wrapper.hpp>

#include <blackhole/extensions/facade.hpp>
#include <blackhole/handler.hpp>
#include <blackhole/root.hpp>

#include "mocks/logger.hpp"
#include "mocks/scope/manager.hpp"

namespace blackhole {
namespace testing {

using ::testing::AtLeast;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SaveArg;
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

TEST(wrapper_t, DelegatesScopeManager) {
    mock::scope::manager_t manager;

    mock::logger_t logger;
    wrapper_t wrapper(logger, {});

    EXPECT_CALL(logger, manager())
        .Times(1)
        .WillOnce(ReturnRef(manager));

    scope::watcher_t* watcher = nullptr;

    EXPECT_CALL(manager, get())
        .Times(AtLeast(1))
        .WillOnce(Return(nullptr))
        .WillOnce(Invoke([&]() -> scope::watcher_t* {
            return watcher;
        }));

    EXPECT_CALL(manager, reset(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&watcher));

    EXPECT_CALL(manager, reset(nullptr))
        .Times(1);

    const scope::holder_t scoped(wrapper, {});
}

}  // namespace testing
}  // namespace blackhole
