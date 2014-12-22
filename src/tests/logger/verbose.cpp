#include <blackhole/logger.hpp>

#include "../global.hpp"
#include "../mocks/frontend.hpp"

using namespace blackhole;

TEST(verbose_logger_t, Constructor) {
    verbose_logger_t<testing::level> log(testing::debug);

    EXPECT_EQ(testing::debug, log.verbosity());
}

namespace { enum class severity { debug, info, warn, error }; }

TEST(verbose_logger_t, ConstructorEnumClass) {
    verbose_logger_t<severity> log(severity::debug);

    EXPECT_EQ(severity::debug, log.verbosity());
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
filter_by_tracebit(const attribute::combined_view_t& view, testing::level severity) {
    auto passed = severity >= testing::warn;

    if (!passed) {
        if (auto tracebit = view.get<std::uint32_t>("tracebit")) {
            return *tracebit == 1;
        }
    }

    return passed;
}

} // namespace

TEST(verbose_logger_t, Verbosity) {
    verbose_logger_t<testing::level> log(testing::debug);
    log.set_filter(testing::warn);

    EXPECT_EQ(testing::warn, log.verbosity());
}

TEST(verbose_logger_t, VerbosityStrongEnum) {
    verbose_logger_t<severity> log(severity::debug);
    log.set_filter(severity::warn);

    EXPECT_EQ(severity::warn, log.verbosity());
}

TEST(verbose_logger_t, ExtendedVerbosity) {
    verbose_logger_t<testing::level> log(testing::debug);
    log.set_filter(testing::warn, &filter_by_tracebit);

    EXPECT_EQ(testing::warn, log.verbosity());
}

TEST(verbose_logger_t, PrimaryVerbosityFiltering) {
    std::unique_ptr<mock::frontend_t> frontend;

    verbose_logger_t<testing::level> log(testing::debug);
    log.add_frontend(std::move(frontend));
    log.set_filter(testing::warn);

    EXPECT_FALSE(log.open_record(testing::debug));
    EXPECT_FALSE(log.open_record(testing::info));
    EXPECT_TRUE (log.open_record(testing::warn));
    EXPECT_TRUE (log.open_record(testing::error));
}

TEST(verbose_logger_t, PrimaryComplexFiltering) {
    std::unique_ptr<mock::frontend_t> frontend;

    verbose_logger_t<testing::level> log(testing::debug);
    log.add_frontend(std::move(frontend));

    log.set_filter(level::debug, &filter_by_tracebit);

    EXPECT_FALSE(log.open_record(testing::debug));
    EXPECT_FALSE(log.open_record(testing::info));
    EXPECT_TRUE (log.open_record(testing::warn));
    EXPECT_TRUE (log.open_record(testing::error));

    {
        const attribute::set_t wrapped = { attribute::make("tracebit", std::uint32_t(0)) };
        EXPECT_FALSE(log.open_record(testing::debug, wrapped));
        EXPECT_FALSE(log.open_record(testing::info, wrapped));
        EXPECT_TRUE (log.open_record(testing::warn, wrapped));
        EXPECT_TRUE (log.open_record(testing::error, wrapped));
    }

    {
        const attribute::set_t wrapped = { attribute::make("tracebit", std::uint32_t(1)) };
        EXPECT_TRUE(log.open_record(testing::debug, wrapped));
        EXPECT_TRUE(log.open_record(testing::info, wrapped));
        EXPECT_TRUE(log.open_record(testing::warn, wrapped));
        EXPECT_TRUE(log.open_record(testing::error, wrapped));
    }
}
