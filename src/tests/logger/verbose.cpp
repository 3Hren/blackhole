#include <blackhole/logger.hpp>

#include "../global.hpp"
#include "../mocks/frontend.hpp"

using namespace blackhole;

TEST(verbose_logger_t, Constructor) {
    verbose_logger_t<testing::level> log(testing::debug);

    EXPECT_EQ(testing::debug, log.verbosity());
}

TEST(verbose_logger_t, MoveExplicitConstructor) {
    verbose_logger_t<testing::level> log(testing::info);
    verbose_logger_t<testing::level> other(std::move(log));

    EXPECT_EQ(testing::info, other.verbosity());
}

TEST(verbose_logger_t, MoveImplicitConstructor) {
    verbose_logger_t<testing::level> log(testing::info);
    verbose_logger_t<testing::level> other = std::move(log);

    EXPECT_EQ(testing::info, other.verbosity());
}

TEST(verbose_logger_t, MoveAssignment) {
    verbose_logger_t<testing::level> log(testing::info);
    verbose_logger_t<testing::level> other(testing::debug);

    other = std::move(log);
    EXPECT_EQ(testing::info, other.verbosity());
}

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

    verbose_logger_t<testing::level> log(testing::debug);
    log.add_frontend(std::move(frontend));
    log.verbosity(testing::warn);

    EXPECT_FALSE(log.open_record(testing::debug).valid());
    EXPECT_FALSE(log.open_record(testing::info).valid());
    EXPECT_TRUE (log.open_record(testing::warn).valid());
    EXPECT_TRUE (log.open_record(testing::error).valid());
}

TEST(verbose_logger_t, PrimaryComplexFiltering) {
    std::unique_ptr<mock::frontend_t> frontend;

    verbose_logger_t<testing::level> log(testing::debug);
    log.add_frontend(std::move(frontend));
    log.verbosity(&filter_by_tracebit);

    EXPECT_FALSE(log.open_record(testing::debug).valid());
    EXPECT_FALSE(log.open_record(testing::info).valid());
    EXPECT_TRUE (log.open_record(testing::warn).valid());
    EXPECT_TRUE (log.open_record(testing::error).valid());

    {
        const attribute::set_t wrapped = { attribute::make("tracebit", std::uint32_t(0)) };
        EXPECT_FALSE(log.open_record(testing::debug, wrapped).valid());
        EXPECT_FALSE(log.open_record(testing::info, wrapped).valid());
        EXPECT_TRUE (log.open_record(testing::warn, wrapped).valid());
        EXPECT_TRUE (log.open_record(testing::error, wrapped).valid());
    }

    {
        const attribute::set_t wrapped = { attribute::make("tracebit", std::uint32_t(1)) };
        EXPECT_TRUE(log.open_record(testing::debug, wrapped).valid());
        EXPECT_TRUE(log.open_record(testing::info, wrapped).valid());
        EXPECT_TRUE(log.open_record(testing::warn, wrapped).valid());
        EXPECT_TRUE(log.open_record(testing::error, wrapped).valid());
    }
}

// TODO: Test secondary filter.

// TODO: Shouldn't compile!!!
TEST(verbose_logger_t, ImportsOpenRecordFromAncestor) {
    verbose_logger_t<testing::level> log(testing::debug);
    log.open_record(blackhole::attribute::set_t({
        blackhole::attribute::make("key", 42)
    }));
}
