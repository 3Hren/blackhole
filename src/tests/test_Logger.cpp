#include "Mocks.hpp"

using namespace blackhole;

TEST(logger_base_t, CanBeEnabled) {
    logger_base_t log;
    log.enable();
    EXPECT_TRUE(log.enabled());
}

TEST(logger_base_t, CanBeDisabled) {
    logger_base_t log;
    log.disable();
    EXPECT_FALSE(log.enabled());
}

TEST(logger_base_t, EnabledByDefault) {
    logger_base_t log;
    EXPECT_TRUE(log.enabled());
}

TEST(logger_base_t, OpenRecordByDefault) {
    logger_base_t log;
    EXPECT_TRUE(log.open_record().valid());
}

TEST(logger_base_t, DoNotOpenRecordIfDisabled) {
    logger_base_t log;
    log.disable();
    EXPECT_FALSE(log.open_record().valid());
}

namespace blackhole { namespace keyword { DECLARE_KEYWORD(urgent, std::uint8_t) } }

TEST(logger_base_t, OpensRecordWhenAttributeFilterSucceed) {
    logger_base_t log;
    log.set_filter(expr::has_attr(keyword::urgent()));
    log.add_attribute(keyword::urgent() = 1);
    EXPECT_TRUE(log.open_record().valid());
}

TEST(logger_base_t, DoNotOpenRecordWhenAttributeFilterFailed) {
    logger_base_t log;
    log.set_filter(expr::has_attr(keyword::urgent()));
    EXPECT_FALSE(log.open_record().valid());
}

TEST(logger_base_t, OpenRecordWhenComplexFilterSucceed) {
    logger_base_t log;
    log.set_filter(expr::has_attr(keyword::urgent()) && keyword::urgent() == 1);
    log.add_attribute(keyword::urgent() = 1);
    EXPECT_TRUE(log.open_record().valid());
}

TEST(logger_base_t, DoNotOpenRecordWhenComplexFilterFailed) {
    logger_base_t log;
    log.set_filter(expr::has_attr(keyword::urgent()) && keyword::urgent() == 1);
    log.add_attribute(keyword::urgent() = 2);
    EXPECT_FALSE(log.open_record().valid());
}
