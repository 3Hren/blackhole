#include <blackhole/macro.hpp>

#include "global.hpp"
#include "mocks/logger.hpp"

using namespace blackhole;

TEST(Macro, OpensInvalidLogRecordAndNotPush) {
    record_t record;
    mock::verbose_log_t<level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));
    EXPECT_CALL(log, push(_))
            .Times(0);
    BH_LOG(log, level::debug, "message");
}

struct ExtractMessageAttributeAction {
    std::string& actual;

    void operator()(record_t record) const {
        actual = record.extract<std::string>("message");
    }
};

TEST(Macro, OpensValidRecordAndPush) {
    record_t record;
    record.insert(attribute::make("attr1", "value1"));

    mock::verbose_log_t<level> log;
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
    record_t record;
    record.insert(attribute::make("attr1", "value1"));

    mock::verbose_log_t<level> log;
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

struct ExtractAttributesAction {
    struct pack_t {
        std::string message;
        int value;
        std::string reason;
        timeval timestamp;
    };

    pack_t& actual;

    void operator ()(record_t record) const {
        actual.message = record.extract<std::string>("message");
        actual.value = record.extract<int>("value");
        actual.reason = record.extract<std::string>("reason");
        actual.timestamp = record.extract<timeval>("timestamp");
    }
};

TEST(Macro, FormatMessageWithAttributes) {
    record_t record;
    record.insert(attribute::make("attr1", "value1"));

    mock::verbose_log_t<level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));

    ExtractAttributesAction::pack_t actual;
    ExtractAttributesAction action { actual };
    EXPECT_CALL(log, push(_))
            .Times(1)
            .WillOnce(WithArg<0>(Invoke(action)));

    BH_LOG(log, level::debug, "message")(
        attribute::make("value", 42),
        attribute::make("reason", "42"),
        keyword::timestamp() = timeval{ 100500, 0 }
    );

    EXPECT_EQ("message", actual.message);
    EXPECT_EQ(42, actual.value);
    EXPECT_EQ("42", actual.reason);
    EXPECT_EQ(100500, actual.timestamp.tv_sec);
}

TEST(Macro, FormatMessageWithPrintfStyleWithAttributes) {
    record_t record;
    record.insert(attribute::make("attr1", "value1"));

    mock::verbose_log_t<level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));

    ExtractAttributesAction::pack_t actual;
    ExtractAttributesAction action { actual };
    EXPECT_CALL(log, push(_))
            .Times(1)
            .WillOnce(WithArg<0>(Invoke(action)));

    BH_LOG(log, level::debug, "value [%d]: %s - okay", 100500, "blah")(
        attribute::make("value", 42),
        attribute::make("reason", "42"),
        keyword::timestamp() = timeval{ 100500, 0 }
    );

    EXPECT_EQ("value [100500]: blah - okay", actual.message);
    EXPECT_EQ(42, actual.value);
    EXPECT_EQ("42", actual.reason);
    EXPECT_EQ(100500, actual.timestamp.tv_sec);
}

struct EmplaceCheckExtractAttributesAction {
    struct pack_t {
        std::string message;
        int value;
        std::string reason;
    };

    pack_t& actual;

    void operator ()(record_t record) const {
        actual.message = record.extract<std::string>("message");
        actual.value = record.extract<int>("value");
        actual.reason = record.extract<std::string>("reason");
    }
};

TEST(Macro, EmplaceAttributes) {
    record_t record;
    record.insert(attribute::make("attr1", "value1"));

    mock::verbose_log_t<level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));

    EmplaceCheckExtractAttributesAction::pack_t actual;
    EmplaceCheckExtractAttributesAction action { actual };
    EXPECT_CALL(log, push(_))
            .Times(1)
            .WillOnce(WithArg<0>(Invoke(action)));

    BH_LOG(log, level::debug, "message")(
        "value", 42,
        "reason", "42"
    );

    EXPECT_EQ("message", actual.message);
    EXPECT_EQ(42, actual.value);
    EXPECT_EQ("42", actual.reason);
}

namespace testing {

struct streamable_value_t {
    std::string message;
    int value;

    friend std::ostream& operator<<(std::ostream& stream, const streamable_value_t& value) {
        stream << "['" << value.message << "', " << value.value << "]";
        return stream;
    }
};

} // namespace testing

struct ExtractStreamableValueAttributesAction {
    struct pack_t {
        std::string message;
        std::string value;
    };

    pack_t& actual;

    void operator()(const record_t& record) const {
        actual.message = record.extract<std::string>("message");
        actual.value = record.extract<std::string>("value");
    }
};

TEST(Macro, UsingStreamOperatorIfNoImplicitConversionAvailable) {
    static_assert(traits::supports::stream_push<streamable_value_t>::value,
                  "`streamable_value_t` must support stream push operator<<");

    streamable_value_t value = { "42", 42 };

    record_t record;
    record.insert(attribute::make("attr1", "value1"));

    mock::verbose_log_t<level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));

    ExtractStreamableValueAttributesAction::pack_t actual;
    ExtractStreamableValueAttributesAction action { actual };
    EXPECT_CALL(log, push(_))
            .Times(1)
            .WillOnce(WithArg<0>(Invoke(action)));

    BH_LOG(log, level::debug, "message")("value", value);

    EXPECT_EQ("message", actual.message);
    EXPECT_EQ("['42', 42]", actual.value);
}

TEST(Macro, InitializerListAttributes) {
    record_t record;
    record.insert(attribute::make("attr1", "value1"));

    mock::verbose_log_t<level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));

    ExtractStreamableValueAttributesAction::pack_t actual;
    ExtractStreamableValueAttributesAction action { actual };
    EXPECT_CALL(log, push(_))
            .Times(1)
            .WillOnce(WithArg<0>(Invoke(action)));

    BH_LOG(log, level::debug, "message")(attribute::list({
        {"value", "42"}
    }));

    EXPECT_EQ("message", actual.message);
    EXPECT_EQ("42", actual.value);
}

namespace RecursiveAttributeFeeders {

struct action_t {
    struct pack_t {
        std::string message;
        std::string value;
        int nested;
    };

    pack_t& actual;

    void operator()(const record_t& record) const {
        actual.message = record.extract<std::string>("message");
        actual.value = record.extract<std::string>("value");
        actual.nested = record.extract<int>("nested");
    }
};

} // namespace RecursiveAttributeFeeders

TEST(Macro, RecursiveAttributeFeeders) {
    record_t record;
    record.insert(attribute::make("attr1", "value1"));

    mock::verbose_log_t<level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));

    RecursiveAttributeFeeders::action_t::pack_t actual;
    RecursiveAttributeFeeders::action_t action { actual };
    EXPECT_CALL(log, push(_))
            .Times(1)
            .WillOnce(WithArg<0>(Invoke(action)));

    BH_LOG(log, level::debug, "message")("value", "42")("nested", 42);

    EXPECT_EQ("message", actual.message);
    EXPECT_EQ("42", actual.value);
    EXPECT_EQ(42, actual.nested);
}

#pragma GCC diagnostic ignored "-Wformat"
#ifndef BLACKHOLE_HAS_GCC44
#pragma GCC diagnostic push
#endif
TEST(Macro, CatchExceptionsOnFormatArgumentsUnderflow) {
    record_t record;
    record.insert(attribute::make("id", "42"));

    mock::verbose_log_t<level> log;
    EXPECT_CALL(log, open_record(level::debug))
            .Times(1)
            .WillOnce(Return(record));

    EXPECT_CALL(log, push(_))
            .Times(1);
    EXPECT_NO_THROW(BH_LOG(log, level::debug, "id: %d"));
}
#ifndef BLACKHOLE_HAS_GCC44
#pragma GCC diagnostic pop
#endif
