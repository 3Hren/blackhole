#include <blackhole/logger.hpp>

#include "../global.hpp"
#include "../mocks/frontend.hpp"

using namespace blackhole;

namespace {

inline
bool
filter_by_tracebit(testing::level severity, const attribute::combined_view_t& view) {
    auto passed = severity >= testing::warn;

    if (!passed) {
        if (auto tracebit = view.get<std::uint32_t>("tracebit")) {
            return *tracebit == 1;
        }
    }

    return passed;
}

} // namespace

TEST(verbose_logger_t, PrimaryVerbosityFiltering) {
    std::unique_ptr<mock::frontend_t> frontend;

    verbose_logger_t<testing::level> log;
    log.add_frontend(std::move(frontend));
    log.verbosity(testing::warn);

    EXPECT_FALSE(log.open_record(testing::debug).valid());
    EXPECT_FALSE(log.open_record(testing::info).valid());
    EXPECT_TRUE (log.open_record(testing::warn).valid());
    EXPECT_TRUE (log.open_record(testing::error).valid());
}

TEST(verbose_logger_t, PrimaryComplexFiltering) {
    std::unique_ptr<mock::frontend_t> frontend;

    verbose_logger_t<testing::level> log;
    log.add_frontend(std::move(frontend));
    log.verbosity(&filter_by_tracebit);

    EXPECT_FALSE(log.open_record(testing::debug).valid());
    EXPECT_FALSE(log.open_record(testing::info).valid());
    EXPECT_TRUE (log.open_record(testing::warn).valid());
    EXPECT_TRUE (log.open_record(testing::error).valid());

    {
        const attribute::set_t wrapped = {{ "tracebit", attribute_t(std::uint32_t(0)) }};
        EXPECT_FALSE(log.open_record(testing::debug, wrapped).valid());
        EXPECT_FALSE(log.open_record(testing::info, wrapped).valid());
        EXPECT_TRUE (log.open_record(testing::warn, wrapped).valid());
        EXPECT_TRUE (log.open_record(testing::error, wrapped).valid());
    }

    {
        const attribute::set_t wrapped = {{ "tracebit", attribute_t(std::uint32_t(1)) }};
        EXPECT_TRUE(log.open_record(testing::debug, wrapped).valid());
        EXPECT_TRUE(log.open_record(testing::info, wrapped).valid());
        EXPECT_TRUE(log.open_record(testing::warn, wrapped).valid());
        EXPECT_TRUE(log.open_record(testing::error, wrapped).valid());
    }
}
