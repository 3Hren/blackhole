#include <gtest/gtest.h>

#include <blackhole/formatter/string.hpp>

using namespace blackhole;

TEST(string_t, MessagePlaceholder) {
    formatter::string_t formatter("[%(message)s]");

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[value]", writer.result().to_string());
}

TEST(string_t, MessageSeverityPlaceholders) {
    formatter::string_t formatter("[%(severity)s]: %(message)s");

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[0]: value", writer.result().to_string());
}

TEST(string_t, TwoPlaceholdersThreeAttributes) {
    formatter::string_t formatter("[%(severity)s]: %(message)s");

    const string_view message("value");
    const attribute_list attributes{{"key#1", {42}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[0]: value", writer.result().to_string());
}

TEST(string_t, ThrowsExceptionWhenAttributeNotFound) {
    formatter::string_t formatter("[%(severity)s]: %(source)s");

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    EXPECT_THROW(formatter.format(record, writer), std::runtime_error);
}

TEST(string_t, DefaultTimestampMapping) {
    formatter::string_t formatter("%(timestamp)s");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();
    writer_t writer;
    formatter.format(record, writer);

    std::ostringstream stream;
    stream << std::chrono::duration_cast<std::chrono::microseconds>(record.timestamp().time_since_epoch()).count();
    EXPECT_EQ(stream.str(), writer.result().to_string());
}

TEST(string_t, CustomTimestampMapping) {
    const auto fn = [](const record_t::time_point& value, writer_t& writer) {
        writer.inner << "2015";
    };

    formatter::string_t formatter("%(timestamp)s", [](int value, writer_t& writer) {}, std::move(fn));

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("2015", writer.result().to_string());
}

TEST(string_t, CustomSeverityMapping) {
    const auto fn = [](int value, writer_t& writer) {
        writer.inner << "DEBUG(" << value << ")";
    };

    formatter::string_t formatter("%(severity)s", std::move(fn), [](const record_t::time_point& value, writer_t& writer) {});

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("DEBUG(0)", writer.result().to_string());
}

// TEST_STRING(Optional, Keyword) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//     record.insert(attribute::make("id", 42));
//
//     std::string pattern("<%(id::)s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<42>: [le message]", fmt.format(record));
// }

// TEST_STRING(Optional, AbsentKeyword) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//
//     std::string pattern("<%(id::)s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, WithPrefix) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//     record.insert(attribute::make("id", 42));
//
//     std::string pattern("<%(id:.:)s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<.42>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, AbsentWithPrefix) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//
//     std::string pattern("<%(id:.:)s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, WithSuffix) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//     record.insert(attribute::make("id", 42));
//
//     std::string pattern("<%(id::.)s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<42.>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, AbsentWithSuffix) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//
//     std::string pattern("<%(id::.)s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, WithPrefixSuffix) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//     record.insert(attribute::make("id", 42));
//
//     std::string pattern("<%(id:.:.)s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<.42.>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, AbsentWithPrefixSuffix) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//
//     std::string pattern("<%(id:.:.)s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, WithPrefixSuffixParentheses) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//     record.insert(attribute::make("id", 42));
//
//     std::string pattern("<%(id:(:))s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<(42)>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, AbsentWithPrefixSuffixParentheses) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//
//     std::string pattern("<%(id:(:))s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, WithPrefixSuffixReverseParentheses) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//     record.insert(attribute::make("id", 42));
//
//     std::string pattern("<%(id:):()s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<)42(>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, AbsentWithPrefixSuffixReverseParentheses) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//
//     std::string pattern("<%(id:):()s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, WithPrefixSuffixSquareBrackets) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//     record.insert(attribute::make("id", 42));
//
//     std::string pattern("<%(id:[:])s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<[42]>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, AbsentWithPrefixSuffixSquareBrackets) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//
//     std::string pattern("<%(id:[:])s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, WithPrefixSuffixSquareBracketsReversed) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//     record.insert(attribute::make("id", 42));
//
//     std::string pattern("<%(id:]:[)s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<]42[>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Optional, AbsentWithPrefixSuffixSquareBracketsReversed) {
//     record_t record;
//     record.insert(attribute::make("message", "le message"));
//
//     std::string pattern("<%(id:]:[)s>: [%(message)s]");
//     formatter::string_t fmt(pattern);
//     EXPECT_EQ("<>: [le message]", fmt.format(record));
// }
//
// TEST_STRING(Deprecated, FormatVariadicSingle) {
//     record_t record;
//     record.insert({ "uuid", attribute_t("123-456") });
//
//     std::string pattern("[%(...L)s]");
//     formatter::string_t formatter(pattern);
//     EXPECT_EQ("['uuid': 123-456]", formatter.format(record));
// }
//
// TEST_STRING(Deprecated, FormatVariadicEmpty) {
//     record_t record;
//
//     std::string pattern("[%(...L)s]");
//     formatter::string_t formatter(pattern);
//     EXPECT_EQ("[]", formatter.format(record));
// }
//
// TEST_STRING(Deprecated, FormatVariadicMultiple) {
//     record_t record;
//     record.insert({ "id", attribute_t(42) });
//     record.insert({ "uuid", attribute_t("123-456") });
//     std::string pattern("[%(...L)s]");
//     formatter::string_t formatter(pattern);
//     std::string actual = formatter.format(record);
//     EXPECT_TRUE(actual.find("'id': 42") != std::string::npos);
//     EXPECT_TRUE(actual.find("'uuid': 123-456") != std::string::npos);
// }
//
// TEST_STRING(Deprecated, ComplexFormatVariadicMultiple) {
//     attribute::set_t external;
//     external.emplace_back("uuid", attribute_t("123-456"));
//     external.emplace_back("answer", attribute_t(42));
//
//     blackhole::attribute::set_t internal;
//     internal.emplace_back("timestamp", attribute_t("1960-01-01 00:00:00"));
//     internal.emplace_back("level", attribute_t("INFO"));
//     internal.emplace_back(keyword::message() = "le message");
//
//     attribute::set_view_t view(external, std::move(internal));
//     record_t record(std::move(view));
//
//     std::string pattern("[%(timestamp)s] [%(level)s]: %(message)s [%(...L)s]");
//     formatter::string_t formatter(pattern);
//     std::string actual = formatter.format(record);
//     EXPECT_TRUE(
//         "[1960-01-01 00:00:00] [INFO]: le message ['uuid': 123-456, 'answer': 42]" == actual ||
//         "[1960-01-01 00:00:00] [INFO]: le message ['answer': 42, 'uuid': 123-456]" == actual
//     );
// }
//
// TEST_STRING(Variadic, SingleWithPrefix) {
//     record_t record;
//     record.insert({ "uuid", attribute_t("123-456") });
//
//     std::string pattern("%(...:args=:)s");
//     formatter::string_t formatter(pattern);
//     EXPECT_EQ("args=uuid: 123-456", formatter.format(record));
// }
//
// TEST_STRING(Variadic, EmptyWithPrefix) {
//     record_t record;
//
//     std::string pattern("%(...:args=:)s");
//     formatter::string_t formatter(pattern);
//     EXPECT_EQ("", formatter.format(record));
// }
//
// TEST_STRING(Variadic, SingleWithSuffix) {
//     record_t record;
//     record.insert({ "uuid", attribute_t("123-456") });
//
//     std::string pattern("%(...::=args)s");
//     formatter::string_t formatter(pattern);
//     EXPECT_EQ("uuid: 123-456=args", formatter.format(record));
// }
//
// TEST_STRING(Variadic, EmptyWithSuffix) {
//     record_t record;
//
//     std::string pattern("%(...::=args)s");
//     formatter::string_t formatter(pattern);
//     EXPECT_EQ("", formatter.format(record));
// }
//
// TEST_STRING(Variadic, SingleWithPrefixSuffix) {
//     record_t record;
//     record.insert({ "uuid", attribute_t("123-456") });
//
//     std::string pattern("%(...:[:])s");
//     formatter::string_t formatter(pattern);
//     EXPECT_EQ("[uuid: 123-456]", formatter.format(record));
// }
//
// TEST_STRING(Variadic, EmptyWithPrefixSuffix) {
//     record_t record;
//
//     std::string pattern("%(...:[:])s");
//     formatter::string_t formatter(pattern);
//     EXPECT_EQ("", formatter.format(record));
// }
//
// TEST_STRING(Variadic, EmptySeparator) {
//     record_t record;
//
//     std::string pattern("%(...::: | )s");
//     formatter::string_t formatter(pattern);
//     EXPECT_EQ("", formatter.format(record));
// }
//
// TEST_STRING(Variadic, SingleSeparator) {
//     record_t record;
//     record.insert({ "uuid", attribute_t("123-456") });
//
//     std::string pattern("%(...::: | )s");
//     formatter::string_t formatter(pattern);
//     EXPECT_EQ("uuid: 123-456", formatter.format(record));
// }
//
// TEST_STRING(Variadic, MultipleSeparator) {
//     record_t record;
//     record.insert({ "id", attribute_t(42) });
//     record.insert({ "uuid", attribute_t("123-456") });
//
//     std::string pattern("%(...::: | )s");
//     formatter::string_t formatter(pattern);
//     auto actual = formatter.format(record);
//     EXPECT_TRUE(
//         "uuid: 123-456 | id: 42" == actual ||
//         "id: 42 | uuid: 123-456" == actual
//     );
// }
//
// TEST_STRING(Variadic, EmptyPrefixSuffixSeparator) {
//     record_t record;
//
//     std::string pattern("%(...:[:]: | )s");
//     formatter::string_t formatter(pattern);
//     EXPECT_EQ("", formatter.format(record));
// }
//
// TEST_STRING(Variadic, SinglePrefixSuffixSeparator) {
//     record_t record;
//     record.insert({ "uuid", attribute_t("123-456") });
//
//     std::string pattern("%(...:[:]: | )s");
//     formatter::string_t formatter(pattern);
//     EXPECT_EQ("[uuid: 123-456]", formatter.format(record));
// }
//
// TEST_STRING(Variadic, MultiplePrefixSuffixSeparator) {
//     record_t record;
//     record.insert({ "id", attribute_t(42) });
//     record.insert({ "uuid", attribute_t("123-456") });
//
//     std::string pattern("%(...:[:]: | )s");
//     formatter::string_t formatter(pattern);
//     auto actual = formatter.format(record);
//     EXPECT_TRUE(
//         "[uuid: 123-456 | id: 42]" == actual ||
//         "[id: 42 | uuid: 123-456]" == actual
//     );
// }
//
// TEST_STRING(Variadic, SinglePattern) {
//     record_t record;
//     record.insert({ "uuid", attribute_t("123-456") });
//
//     std::string pattern("%(...[%k=%v])s");
//     formatter::string_t formatter(pattern);
//     EXPECT_EQ("uuid=123-456", formatter.format(record));
// }
//
// TEST_STRING(Variadic, MultiplePrefixSuffixSeparatorPattern) {
//     record_t record;
//     record.insert({ "id", attribute_t(42) });
//     record.insert({ "uuid", attribute_t("123-456") });
//
//     std::string pattern("%(...[%k=%v]:[:]: | )s");
//     formatter::string_t formatter(pattern);
//     auto actual = formatter.format(record);
//     EXPECT_TRUE(
//         "[uuid=123-456 | id=42]" == actual ||
//         "[id=42 | uuid=123-456]" == actual
//     );
// }
//
// TEST_STRING(Variadic, MultipleDuplicated) {
//     record_t record;
//     record.insert({ "id", attribute_t(42) });
//     record.insert({ "id", attribute_t(45) });
//
//     std::string pattern("%(...)s");
//     formatter::string_t::config_type config(pattern);
//     config.filter = false;
//     formatter::string_t formatter(config);
//     EXPECT_EQ("id: 42, id: 45", formatter.format(record));
// }
//
// TEST_STRING(Variadic, MultipleDuplicatedWithFilterDuplicatedByDefault) {
//     record_t record;
//     record.insert({ "id", attribute_t(42) });
//     record.insert({ "id", attribute_t(45) });
//
//     std::string pattern("%(...)s");
//     formatter::string_t::config_type config(pattern);
//     formatter::string_t formatter(config);
//     EXPECT_EQ("id: 45", formatter.format(record));
// }
