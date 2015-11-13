#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/logger.hpp>
#include <blackhole/wrapper.hpp>

#include <blackhole/extensions/facade.hpp>
#include <blackhole/handler.hpp>
#include <blackhole/root.hpp>

#include "mocks/logger.hpp"

namespace blackhole {
namespace testing {

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

}  // namespace testing
}  // namespace blackhole
