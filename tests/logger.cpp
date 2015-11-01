#include <gtest/gtest.h>

#include <blackhole/logger.hpp>

namespace blackhole {
namespace testing {

/// Can be initialized with none handlers.
TEST(Logger, Constructor) {
    logger_t log({});

    (void)log;
}

// log.log(0, {{"#1", attribute_value_t("v1")}}, "### {}", 42);

}  // namespace testing
}  // namespace blackhole
