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

struct ExtractMessageAttributeAction {
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
    ExtractMessageAttributeAction action { actual };
    EXPECT_CALL(log, push(_))
            .Times(1)
            .WillOnce(WithArg<0>(Invoke(action)));
    BH_LOG(log, level::debug, "value");

    EXPECT_EQ("value", actual);
}

TEST(Macro, FormatMessageWithPrintfStyle) {
    log::record_t record;
    record.attributes["attr1"] = {"value1"};

    mock::verbose_log_t<testing::level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));

    std::string actual;
    ExtractMessageAttributeAction action { actual };
    EXPECT_CALL(log, push(_))
            .Times(1)
            .WillOnce(WithArg<0>(Invoke(action)));

    BH_LOG(log, level::debug, "value [%d]: %s - okay", 100500, "blah");
    EXPECT_EQ("value [100500]: blah - okay", actual);
}

namespace testing {

struct attr_pack_t {
    std::string message;
    int value;
    std::string reason;
    std::time_t timestamp;
};

} // namespace testing

struct ExtractAttributesAction {
    attr_pack_t& actual;

    void operator ()(log::record_t record) const {
        actual.message = record.extract<std::string>("message");
        actual.value = record.extract<int>("value");
        actual.reason = record.extract<std::string>("reason");
        actual.timestamp = record.extract<std::time_t>("timestamp");
    }
};

TEST(Macro, FormatMessageWithAttributes) {
    log::record_t record;
    record.attributes["attr1"] = {"value1"};

    mock::verbose_log_t<testing::level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));

    testing::attr_pack_t actual;
    ExtractAttributesAction action { actual };
    EXPECT_CALL(log, push(_))
            .Times(1)
            .WillOnce(WithArg<0>(Invoke(action)));

    BH_LOG(log, level::debug, "message")(
        attribute::make("value", 42),
        attribute::make("reason", "42"),
        keyword::timestamp() = 100500
    );

    EXPECT_EQ("message", actual.message);
    EXPECT_EQ(42, actual.value);
    EXPECT_EQ("42", actual.reason);
    EXPECT_EQ(100500, actual.timestamp);
}

TEST(Macro, FormatMessageWithPrintfStyleWithAttributes) {
    log::record_t record;
    record.attributes["attr1"] = {"value1"};

    mock::verbose_log_t<testing::level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));

    testing::attr_pack_t actual;
    ExtractAttributesAction action { actual };
    EXPECT_CALL(log, push(_))
            .Times(1)
            .WillOnce(WithArg<0>(Invoke(action)));

    BH_LOG(log, level::debug, "value [%d]: %s - okay", 100500, "blah")(
        attribute::make("value", 42),
        attribute::make("reason", "42"),
        keyword::timestamp() = 100500
    );

    EXPECT_EQ("value [100500]: blah - okay", actual.message);
    EXPECT_EQ(42, actual.value);
    EXPECT_EQ("42", actual.reason);
    EXPECT_EQ(100500, actual.timestamp);
}

namespace blackhole {

namespace format {

namespace message {

template<>
struct insitu<keyword::tag::severity_t<level>> {
    static inline std::ostream& execute(std::ostream& stream, level lvl) {
        static std::string DESCRIPTIONS[] = {
            "DEBUG",
            "INFO ",
            "WARN ",
            "ERROR"
        };

        if (static_cast<std::size_t>(lvl) < sizeof(DESCRIPTIONS) / sizeof(DESCRIPTIONS[0])) {
            stream << DESCRIPTIONS[lvl];
        } else {
            stream << static_cast<int>(lvl);
        }

        return stream;
    }
};

} // namespace message

} // namespace format

} // namespace blackhole

TEST(Macro, SpecificKeywordMessageFormatting) {
    log::record_t record;
    record.attributes = {
        keyword::severity<level>() = level::debug
    };

    mock::verbose_log_t<testing::level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));

    std::string actual;
    ExtractMessageAttributeAction action { actual };
    EXPECT_CALL(log, push(_))
            .Times(1)
            .WillOnce(WithArg<0>(Invoke(action)));
    BH_LOG(log, level::debug, "value: %s", keyword::severity<level>());

    EXPECT_EQ("value: DEBUG", actual);
}
