#include "Mocks.hpp"

using namespace blackhole;

namespace testing {

enum level { debug, info, warn, error };

} // namespace testing

TEST(Macro, OpensInvalidLogRecordAndNotPush) {
    log::record_t record;
    mock::verbose_log_t<testing::level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));
    EXPECT_CALL(log, push(_))
            .Times(0);
    BH_LOG(log, level::debug, "message");
}

struct ExtractMessageAttribute {
    std::string& actual;

    void operator ()(log::record_t record) const {
        actual = record.extract<std::string>("message");
    }
};

TEST(Macro, OpensValidRecordAndPush) {
    log::record_t record;
    record.attributes["attr1"] = {"value1"};

    mock::verbose_log_t<testing::level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));

    std::string actual;
    ExtractMessageAttribute action { actual };
    EXPECT_CALL(log, push(_))
            .Times(1)
            .WillOnce(WithArg<0>(Invoke(action)));
    BH_LOG(log, level::debug, "value");

    EXPECT_EQ(actual, "value");
}

TEST(Macro, FormatMessageWithPrintfStyle) {
    log::record_t record;
    record.attributes["attr1"] = {"value1"};

    mock::verbose_log_t<testing::level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));

    std::string actual;
    ExtractMessageAttribute action { actual };
    EXPECT_CALL(log, push(_))
            .Times(1)
            .WillOnce(WithArg<0>(Invoke(action)));

    BH_LOG(log, level::debug, "value [%d]: %s - okay", 100500, "blah");
    EXPECT_EQ(actual, "value [100500]: blah - okay");
}
