#include <gtest/gtest.h>

#include <blackhole/logger.hpp>
#include <blackhole/wrapper.hpp>

namespace blackhole {
namespace testing {

/// Can be initialized with none handlers.
TEST(Logger, Constructor) {
    root_logger_t log({});

    (void)log;
}

TEST(wrapper, call) {
    using attribute::value_t;
    using attribute::owned_t;

    root_logger_t log({});

    wrapper_t wrapper1{log, {
        {"key#0", owned_t(0)},
        {"key#1", owned_t("value#1")}
    }};

    wrapper_t wrapper2{wrapper1, {
        {"key#2", owned_t(2)},
        {"key#3", owned_t("value#3")}
    }};

    wrapper2.log(0,
        {
            {"key#4", value_t(42)},
            {"key#5", value_t(3.1415)},
            {"key#6", value_t("value")}
        }, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
        "[::]",
        "esafronov",
        "10/Oct/2000:13:55:36 -0700",
        "/porn.png",
        200,
        2326
    );
}

}  // namespace testing
}  // namespace blackhole
