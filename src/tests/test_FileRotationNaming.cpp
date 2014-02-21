#include <blackhole/sink/files/rotation/naming/filter.hpp>

#include "global.hpp"

using namespace blackhole::sink::rotation;

TEST(match, MatchCounter) {
    EXPECT_TRUE(naming::aux::matched("test.log.%N", "test.log.1"));
}

TEST(match, MatchCounterInMiddle) {
    EXPECT_TRUE(naming::aux::matched("test.log.%N.log", "test.log.1.log"));
}

TEST(match, NegativeMatchCounterWithWidth) {
    EXPECT_FALSE(naming::aux::matched("test.log.%0N.log", "test.log..log"));
    EXPECT_FALSE(naming::aux::matched("test.log.%0N.log", "test.log.1.log"));

    EXPECT_FALSE(naming::aux::matched("test.log.%1N.log", "test.log.01.log"));
    EXPECT_FALSE(naming::aux::matched("test.log.%1N.log", "test.log.10.log"));
    EXPECT_FALSE(naming::aux::matched("test.log.%2N.log", "test.log.1.log"));
    EXPECT_FALSE(naming::aux::matched("test.log.%2N.log", "test.log.2.log"));
    EXPECT_FALSE(naming::aux::matched("test.log.%2N.log", "test.log.100.log"));
}

TEST(match, PositiveMatchCounterWithWidth) {
    EXPECT_TRUE(naming::aux::matched("test.log.%1N.log", "test.log.1.log"));
    EXPECT_TRUE(naming::aux::matched("test.log.%2N.log", "test.log.01.log"));
    EXPECT_TRUE(naming::aux::matched("test.log.%2N.log", "test.log.10.log"));
    EXPECT_TRUE(naming::aux::matched("test.log.%5N.log", "test.log.00001.log"));
    EXPECT_TRUE(naming::aux::matched("test.log.%5N.log", "test.log.10000.log"));
}

TEST(match, NegativeMatchCounterWithDatetime) {
    EXPECT_FALSE(naming::aux::matched("test.log.%N.%Y%m%d.log", "test.log...log"));

    EXPECT_FALSE(naming::aux::matched("test.log.%N.%Y%m%d.log", "test.log..20140101.log"));

    EXPECT_FALSE(naming::aux::matched("test.log.%N.%Y%m%d.log", "test.log.01.20140101.log"));
    EXPECT_FALSE(naming::aux::matched("test.log.%N.%Y%m%d.log", "test.log.10.20140101.log"));

    EXPECT_FALSE(naming::aux::matched("test.log.%N.%Y%m%d.log", "test.log.1.201401--.log"));
    EXPECT_FALSE(naming::aux::matched("test.log.%N.%Y%m%d.log", "test.log.1.120140101.log"));
}

TEST(match, PositiveMatchCounterWithDatetime) {
    EXPECT_TRUE(naming::aux::matched("test.log.%N.%Y%m%d.log", "test.log.1.20140101.log"));
}
