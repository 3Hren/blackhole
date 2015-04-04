#include <blackhole/formatter/string.hpp>
#include <blackhole/keyword/message.hpp>
#include <blackhole/keyword/timestamp.hpp>

#include "../global.hpp"

using namespace blackhole;

#define TEST_STRING(Suite, Case) \
    TEST(string_t##_##Suite, Case)

TEST_STRING(Required, SinglePlaceholder) {
    record_t record;
    record.insert(keyword::message() = "le message");

    std::string pattern("[%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("[le message]", fmt.format(record));
}

TEST_STRING(Required, TwoPlaceholders) {
    record_t record;
    record.insert(keyword::message() = "le message");
    record.insert({ "timestamp", attribute_t("le timestamp") });

    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("[le timestamp]: le message", fmt.format(record));
}

TEST_STRING(Required, TwoPlaceholdersThreeAttributes) {
    record_t record;
    record.insert(keyword::message() = "le message");
    record.insert({ "timestamp", attribute_t("le timestamp") });
    record.insert({ "source", attribute_t("le source") });

    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("[le timestamp]: le message", fmt.format(record));
}

TEST_STRING(Required, ThrowsExceptionWhenAttributeNotFound) {
    record_t record;
    record.insert(keyword::message() = "le message");

    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t fmt(pattern);
    EXPECT_THROW(fmt.format(record), blackhole::error_t);
}

namespace testing {

void map_timestamp(blackhole::stickystream_t& stream, const timeval& tv) {
    char str[64];

    struct tm tm;
    gmtime_r((time_t *)&tv.tv_sec, &tm);
    if (std::strftime(str, sizeof(str), "%F %T", &tm)) {
        char usecs[64];
        snprintf(usecs, sizeof(usecs), ".%06ld", (long)tv.tv_usec);
        stream << str << usecs;
    } else {
        stream << "UNKNOWN";
    }
}

} // namespace testing

TEST(string_t, CustomMapping) {
    mapping::value_t mapper;
    mapper.add<timeval>("timestamp", &testing::map_timestamp);

    record_t record;
    record.insert(keyword::timestamp() = timeval { 100500, 0 });
    record.insert(keyword::message() = "le message");

    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t formatter(pattern);
    formatter.set_mapper(std::move(mapper));
    std::string actual = formatter.format(record);
    EXPECT_EQ(actual, "[1970-01-02 03:55:00.000000]: le message");
}

TEST(string_t, CustomMappingWithKeyword) {
    mapping::value_t mapper;
    mapper.add<keyword::tag::timestamp_t>(&testing::map_timestamp);

    record_t record;
    record.insert(keyword::timestamp() = timeval { 100500, 0 });
    record.insert(keyword::message() = "le message");

    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t formatter(pattern);
    formatter.set_mapper(std::move(mapper));
    std::string actual = formatter.format(record);
    EXPECT_EQ(actual, "[1970-01-02 03:55:00.000000]: le message");
}

TEST(mapping, DatetimeMapping) {
    mapping::value_t mapper;
    mapper.add<keyword::tag::timestamp_t>("%Y-%m-%d %H:%M:%S");

    std::string result;
    stickystream_t stream;
    stream.attach(result);
    timeval tv{ 100500, 0 };
    mapper(stream, "timestamp", tv);
    EXPECT_EQ("1970-01-02 03:55:00", result);
}

TEST_STRING(Optional, Keyword) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%(id::)s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<42>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, AbsentKeyword) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%(id::)s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, WithPrefix) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%(id:.:)s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<.42>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, AbsentWithPrefix) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%(id:.:)s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, WithSuffix) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%(id::.)s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<42.>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, AbsentWithSuffix) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%(id::.)s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, WithPrefixSuffix) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%(id:.:.)s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<.42.>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, AbsentWithPrefixSuffix) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%(id:.:.)s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, WithPrefixSuffixParentheses) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%(id:(:))s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<(42)>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, AbsentWithPrefixSuffixParentheses) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%(id:(:))s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, WithPrefixSuffixReverseParentheses) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%(id:):()s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<)42(>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, AbsentWithPrefixSuffixReverseParentheses) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%(id:):()s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, WithPrefixSuffixSquareBrackets) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%(id:[:])s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<[42]>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, AbsentWithPrefixSuffixSquareBrackets) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%(id:[:])s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, WithPrefixSuffixSquareBracketsReversed) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%(id:]:[)s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<]42[>: [le message]", fmt.format(record));
}

TEST_STRING(Optional, AbsentWithPrefixSuffixSquareBracketsReversed) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%(id:]:[)s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING(Deprecated, FormatVariadicSingle) {
    record_t record;
    record.insert({ "uuid", attribute_t("123-456") });

    std::string pattern("[%(...L)s]");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("['uuid': 123-456]", formatter.format(record));
}

TEST_STRING(Deprecated, FormatVariadicEmpty) {
    record_t record;

    std::string pattern("[%(...L)s]");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("[]", formatter.format(record));
}

TEST_STRING(Deprecated, FormatVariadicMultiple) {
    record_t record;
    record.insert({ "id", attribute_t(42) });
    record.insert({ "uuid", attribute_t("123-456") });
    std::string pattern("[%(...L)s]");
    formatter::string_t formatter(pattern);
    std::string actual = formatter.format(record);
    EXPECT_TRUE(actual.find("'id': 42") != std::string::npos);
    EXPECT_TRUE(actual.find("'uuid': 123-456") != std::string::npos);
}

TEST_STRING(Deprecated, ComplexFormatVariadicMultiple) {
    attribute::set_t external;
    external.emplace_back("uuid", attribute_t("123-456"));
    external.emplace_back("answer", attribute_t(42));

    blackhole::attribute::set_t internal;
    internal.emplace_back("timestamp", attribute_t("1960-01-01 00:00:00"));
    internal.emplace_back("level", attribute_t("INFO"));
    internal.emplace_back(keyword::message() = "le message");

    attribute::set_view_t view(external, std::move(internal));
    record_t record(std::move(view));

    std::string pattern("[%(timestamp)s] [%(level)s]: %(message)s [%(...L)s]");
    formatter::string_t formatter(pattern);
    std::string actual = formatter.format(record);
    EXPECT_TRUE(
        "[1960-01-01 00:00:00] [INFO]: le message ['uuid': 123-456, 'answer': 42]" == actual ||
        "[1960-01-01 00:00:00] [INFO]: le message ['answer': 42, 'uuid': 123-456]" == actual
    );
}

TEST_STRING(Variadic, SingleWithPrefix) {
    record_t record;
    record.insert({ "uuid", attribute_t("123-456") });

    std::string pattern("%(...:args=:)s");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("args=uuid: 123-456", formatter.format(record));
}

TEST_STRING(Variadic, EmptyWithPrefix) {
    record_t record;

    std::string pattern("%(...:args=:)s");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("", formatter.format(record));
}

TEST_STRING(Variadic, SingleWithSuffix) {
    record_t record;
    record.insert({ "uuid", attribute_t("123-456") });

    std::string pattern("%(...::=args)s");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("uuid: 123-456=args", formatter.format(record));
}

TEST_STRING(Variadic, EmptyWithSuffix) {
    record_t record;

    std::string pattern("%(...::=args)s");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("", formatter.format(record));
}

TEST_STRING(Variadic, SingleWithPrefixSuffix) {
    record_t record;
    record.insert({ "uuid", attribute_t("123-456") });

    std::string pattern("%(...:[:])s");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("[uuid: 123-456]", formatter.format(record));
}

TEST_STRING(Variadic, EmptyWithPrefixSuffix) {
    record_t record;

    std::string pattern("%(...:[:])s");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("", formatter.format(record));
}

TEST_STRING(Variadic, EmptySeparator) {
    record_t record;

    std::string pattern("%(...::: | )s");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("", formatter.format(record));
}

TEST_STRING(Variadic, SingleSeparator) {
    record_t record;
    record.insert({ "uuid", attribute_t("123-456") });

    std::string pattern("%(...::: | )s");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("uuid: 123-456", formatter.format(record));
}

TEST_STRING(Variadic, MultipleSeparator) {
    record_t record;
    record.insert({ "id", attribute_t(42) });
    record.insert({ "uuid", attribute_t("123-456") });

    std::string pattern("%(...::: | )s");
    formatter::string_t formatter(pattern);
    auto actual = formatter.format(record);
    EXPECT_TRUE(
        "uuid: 123-456 | id: 42" == actual ||
        "id: 42 | uuid: 123-456" == actual
    );
}

TEST_STRING(Variadic, EmptyPrefixSuffixSeparator) {
    record_t record;

    std::string pattern("%(...:[:]: | )s");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("", formatter.format(record));
}

TEST_STRING(Variadic, SinglePrefixSuffixSeparator) {
    record_t record;
    record.insert({ "uuid", attribute_t("123-456") });

    std::string pattern("%(...:[:]: | )s");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("[uuid: 123-456]", formatter.format(record));
}

TEST_STRING(Variadic, MultiplePrefixSuffixSeparator) {
    record_t record;
    record.insert({ "id", attribute_t(42) });
    record.insert({ "uuid", attribute_t("123-456") });

    std::string pattern("%(...:[:]: | )s");
    formatter::string_t formatter(pattern);
    auto actual = formatter.format(record);
    EXPECT_TRUE(
        "[uuid: 123-456 | id: 42]" == actual ||
        "[id: 42 | uuid: 123-456]" == actual
    );
}

TEST_STRING(Variadic, SinglePattern) {
    record_t record;
    record.insert({ "uuid", attribute_t("123-456") });

    std::string pattern("%(...[%k=%v])s");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("uuid=123-456", formatter.format(record));
}

TEST_STRING(Variadic, MultiplePrefixSuffixSeparatorPattern) {
    record_t record;
    record.insert({ "id", attribute_t(42) });
    record.insert({ "uuid", attribute_t("123-456") });

    std::string pattern("%(...[%k=%v]:[:]: | )s");
    formatter::string_t formatter(pattern);
    auto actual = formatter.format(record);
    EXPECT_TRUE(
        "[uuid=123-456 | id=42]" == actual ||
        "[id=42 | uuid=123-456]" == actual
    );
}

TEST_STRING(Variadic, MultipleDuplicated) {
    record_t record;
    record.insert({ "id", attribute_t(42) });
    record.insert({ "id", attribute_t(45) });

    std::string pattern("%(...)s");
    formatter::string_t::config_type config(pattern);
    config.filter = false;
    formatter::string_t formatter(config);
    EXPECT_EQ("id: 42, id: 45", formatter.format(record));
}

TEST_STRING(Variadic, MultipleDuplicatedWithFilterDuplicatedByDefault) {
    record_t record;
    record.insert({ "id", attribute_t(42) });
    record.insert({ "id", attribute_t(45) });

    std::string pattern("%(...)s");
    formatter::string_t::config_type config(pattern);
    formatter::string_t formatter(config);
    EXPECT_EQ("id: 45", formatter.format(record));
}
