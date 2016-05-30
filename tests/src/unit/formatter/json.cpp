#include <array>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <rapidjson/document.h>

#include <blackhole/attribute.hpp>
#include <blackhole/attributes.hpp>
#include <blackhole/cpp17/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter.hpp>
#include <blackhole/formatter/json.hpp>
#include <blackhole/record.hpp>
#include <src/formatter/json.hpp>

#include "mocks/node.hpp"

namespace {

struct endpoint_t {
    std::string host;
    std::uint16_t port;
};

}  // namespace

namespace blackhole {
inline namespace v1 {

template<>
struct display_traits<endpoint_t> {
    static auto apply(const endpoint_t& endpoint, writer_t& wr) -> void {
        wr.write("{}:{}", endpoint.host, endpoint.port);
    }
};

}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace formatter {
namespace {

using ::testing::StrictMock;
using ::testing::Return;

using experimental::builder;
using experimental::factory;

TEST(json_t, FormatMessage) {
    auto formatter = builder<json_t>()
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("message"));
    ASSERT_TRUE(doc["message"].IsString());
    EXPECT_STREQ("value", doc["message"].GetString());
}

TEST(json_t, FormatFormattedMessage) {
    auto formatter = builder<json_t>()
        .build();

    const string_view message("value {}");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate("value 42");
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("message"));
    ASSERT_TRUE(doc["message"].IsString());
    EXPECT_STREQ("value 42", doc["message"].GetString());
}

TEST(json_t, FormatSeverity) {
    auto formatter = builder<json_t>()
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(4, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());

    ASSERT_TRUE(doc.HasMember("severity"));
    ASSERT_TRUE(doc["severity"].IsInt());
    EXPECT_EQ(4, doc["severity"].GetInt());
}

TEST(json_t, FormatTimestamp) {
    auto formatter = builder<json_t>()
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(4, message, pack);
    record.activate();
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());

    ASSERT_TRUE(doc.HasMember("timestamp"));
    ASSERT_TRUE(doc["timestamp"].IsUint64());
    EXPECT_TRUE(doc["timestamp"].GetUint64() > 0);
}

TEST(json_t, FormatProcess) {
    auto formatter = builder<json_t>()
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(4, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());

    ASSERT_TRUE(doc.HasMember("process"));
    ASSERT_TRUE(doc["process"].IsInt());
    EXPECT_EQ(::getpid(), doc["process"].GetInt());
}

TEST(json_t, FormatThread) {
    auto formatter = builder<json_t>()
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(4, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());

    ASSERT_TRUE(doc.HasMember("thread"));
    ASSERT_TRUE(doc["thread"].IsString());

    std::ostringstream stream;
#ifdef __linux
    stream << std::hex << std::internal << std::showbase << std::setw(2) << std::setfill('0');
#endif
    stream << std::this_thread::get_id();

    EXPECT_EQ(stream.str(), doc["thread"].GetString());
}

TEST(json_t, FormatAttribute) {
    auto formatter = builder<json_t>()
        .build();

    const string_view message("value");
    const attribute_list attributes{{"counter", 42}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("message"));
    ASSERT_TRUE(doc["message"].IsString());
    EXPECT_STREQ("value", doc["message"].GetString());

    ASSERT_TRUE(doc.HasMember("counter"));
    ASSERT_TRUE(doc["counter"].IsInt());
    EXPECT_EQ(42, doc["counter"].GetInt());
}

TEST(json_t, FormatAttributeNull) {
    auto formatter = builder<json_t>()
        .build();

    const string_view message("value");
    const attribute_list attributes{{"endpoint", nullptr}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("endpoint"));
    ASSERT_TRUE(doc["endpoint"].IsNull());
}

TEST(json_t, FormatAttributeBool) {
    auto formatter = builder<json_t>()
        .build();

    const string_view message("value");
    const attribute_list attributes{{"available", true}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("available"));
    ASSERT_TRUE(doc["available"].IsBool());
    EXPECT_TRUE(doc["available"].GetBool());
}

TEST(json_t, FormatAttributeDouble) {
    auto formatter = builder<json_t>()
        .build();

    const string_view message("value");
    const attribute_list attributes{{"pi", 3.1415}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("pi"));
    ASSERT_TRUE(doc["pi"].IsDouble());
    EXPECT_DOUBLE_EQ(3.1415, doc["pi"].GetDouble());
}

TEST(json_t, FormatAttributeString) {
    auto formatter = builder<json_t>()
        .build();

    const string_view message("value");
    const attribute_list attributes{{"endpoint", "127.0.0.1:8080"}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("message"));
    ASSERT_TRUE(doc["message"].IsString());
    EXPECT_STREQ("value", doc["message"].GetString());

    ASSERT_TRUE(doc.HasMember("endpoint"));
    ASSERT_TRUE(doc["endpoint"].IsString());
    EXPECT_STREQ("127.0.0.1:8080", doc["endpoint"].GetString());
}

TEST(json_t, FormatAttributeUser) {
    endpoint_t endpoint{"127.0.0.1", 8080};

    auto formatter = builder<json_t>()
        .build();

    const string_view message("value");
    const attribute_list attributes{{"endpoint", endpoint}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("message"));
    ASSERT_TRUE(doc["message"].IsString());
    EXPECT_STREQ("value", doc["message"].GetString());

    ASSERT_TRUE(doc.HasMember("endpoint"));
    ASSERT_TRUE(doc["endpoint"].IsString());
    EXPECT_STREQ("127.0.0.1:8080", doc["endpoint"].GetString());
}

TEST(json_t, FormatDuplicateAttributesDefault) {
    auto formatter = builder<json_t>()
        .build();

    const string_view message("value");
    const attribute_list a1{{"counter", 42}};
    const attribute_list a2{{"counter", 100}};
    const attribute_pack pack{a1, a2};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("message"));
    ASSERT_TRUE(doc["message"].IsString());
    EXPECT_STREQ("value", doc["message"].GetString());

    ASSERT_TRUE(doc.HasMember("counter"));
    ASSERT_TRUE(doc["counter"].IsInt());
    EXPECT_EQ(42, doc["counter"].GetInt());

    EXPECT_TRUE(writer.result().to_string().find("\"counter\":42") != std::string::npos);
    EXPECT_TRUE(writer.result().to_string().find("\"counter\":100") != std::string::npos);
}

TEST(json_t, FormatDuplicateAttributesUnique) {
    auto formatter = builder<json_t>()
        .unique()
        .build();

    const string_view message("value");
    // Earlier attribute list has more precedence than the later ones.
    const attribute_list a1{{"counter", 42}};
    const attribute_list a2{{"counter", 100}};
    const attribute_pack pack{a1, a2};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("message"));
    ASSERT_TRUE(doc["message"].IsString());
    EXPECT_STREQ("value", doc["message"].GetString());

    ASSERT_TRUE(doc.HasMember("counter"));
    ASSERT_TRUE(doc["counter"].IsInt());
    EXPECT_EQ(42, doc["counter"].GetInt());

    EXPECT_TRUE(writer.result().to_string().find("\"counter\":42") != std::string::npos);
    EXPECT_TRUE(writer.result().to_string().find("\"counter\":100") == std::string::npos);
}

TEST(json_t, FormatMessageWithRouting) {
    auto formatter = builder<json_t>()
        .route("/fields", {"message"})
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("fields"));
    ASSERT_TRUE(doc["fields"].HasMember("message"));
    ASSERT_TRUE(doc["fields"]["message"].IsString());
    EXPECT_STREQ("value", doc["fields"]["message"].GetString());
}

TEST(json_t, FormatAttributeStringWithRouting) {
    auto formatter = builder<json_t>()
        .route("/fields", {"endpoint"})
        .build();

    const string_view message("value");
    const attribute_list attributes{{"endpoint", "127.0.0.1:8080"}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("fields"));
    ASSERT_TRUE(doc["fields"].HasMember("endpoint"));
    ASSERT_TRUE(doc["fields"]["endpoint"].IsString());
    EXPECT_STREQ("127.0.0.1:8080", doc["fields"]["endpoint"].GetString());
}

TEST(json_t, FormatAttributeStringWithNestedRouting) {
    auto formatter = builder<json_t>()
        .route("/fields/external", {"endpoint"})
        .build();

    const string_view message("value");
    const attribute_list attributes{{"endpoint", "127.0.0.1:8080"}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("fields"));
    ASSERT_TRUE(doc["fields"].HasMember("external"));
    ASSERT_TRUE(doc["fields"]["external"].IsObject());
    ASSERT_TRUE(doc["fields"]["external"].HasMember("endpoint"));
    ASSERT_TRUE(doc["fields"]["external"]["endpoint"].IsString());
    EXPECT_STREQ("127.0.0.1:8080", doc["fields"]["external"]["endpoint"].GetString());
}

TEST(json_t, FormatAttributeStringWithRootRouting) {
    auto formatter = builder<json_t>()
        .route("", {"endpoint"})
        .build();

    const string_view message("value");
    const attribute_list attributes{{"endpoint", "127.0.0.1:8080"}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("endpoint"));
    ASSERT_TRUE(doc["endpoint"].IsString());
    EXPECT_STREQ("127.0.0.1:8080", doc["endpoint"].GetString());
}

TEST(json_t, FormatMessageWithRenaming) {
    auto formatter = builder<json_t>()
        .rename("message", "#message")
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("#message"));
    ASSERT_TRUE(doc["#message"].IsString());
    EXPECT_STREQ("value", doc["#message"].GetString());
}

TEST(json_t, FormatAttributeWithRenaming) {
    auto formatter = builder<json_t>()
        .rename("source", "#source")
        .build();

    const string_view message("value");
    const attribute_list attributes{{"source", "storage"}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());
    ASSERT_TRUE(doc.HasMember("#source"));
    ASSERT_TRUE(doc["#source"].IsString());
    EXPECT_STREQ("storage", doc["#source"].GetString());
}

TEST(json_t, FormatMessageWithoutNewlineByDefault) {
    auto formatter = builder<json_t>()
        .build();

    const string_view message("");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_NE('\n', writer.result().to_string().back());
}

TEST(json_t, FormatMessageWithNewline) {
    auto formatter = builder<json_t>()
        .newline()
        .build();

    const string_view message("");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ('\n', writer.result().to_string().back());
}

TEST(json_t, MutateTimestamp) {
    auto formatter = builder<json_t>()
        .timestamp("%Y!")
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(4, message, pack);
    record.activate();
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());

    ASSERT_TRUE(doc.HasMember("timestamp"));
    ASSERT_TRUE(doc["timestamp"].IsString());

    std::array<char, 128> buffer;
    const auto time = record_t::clock_type::to_time_t(record.timestamp());
    std::tm tm;
    ::gmtime_r(&time, &tm);
    const auto size = std::strftime(buffer.data(), buffer.size(), "%Y!", &tm);
    ASSERT_TRUE(size > 0);

    EXPECT_EQ(std::string(buffer.data(), size), std::string(doc["timestamp"].GetString()));
}

TEST(json_t, MutateSeverity) {
    auto formatter = builder<json_t>()
        .severity({"D", "I", "W", "E"})
        .build();

    const string_view message("");
    const attribute_pack pack;
    record_t record(2, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    rapidjson::Document doc;
    doc.Parse<0>(writer.result().to_string().c_str());

    ASSERT_TRUE(doc.HasMember("severity"));
    ASSERT_TRUE(doc["severity"].IsString());
    EXPECT_EQ("W", std::string(doc["severity"].GetString()));
}

TEST(json_t, UniqueDisabledByDefault) {
    EXPECT_FALSE(json_t().unique());
}

TEST(json_t, NoNewlineByDefault) {
    EXPECT_FALSE(json_t().newline());
}

TEST(builder_t, Newline) {
    const auto layout = builder<json_t>()
        .newline()
        .build();

    const auto& cast = dynamic_cast<const json_t&>(*layout);
    EXPECT_TRUE(cast.newline());
}

TEST(builder_t, Unique) {
    auto layout = builder<json_t>()
        .unique()
        .build();

    const auto& cast = dynamic_cast<const json_t&>(*layout);
    EXPECT_TRUE(cast.unique());
}

TEST(json_t, FactoryType) {
    EXPECT_EQ(std::string("json"), factory<json_t>().type());
}

// TODO: Test json factory.

}  // namespace
}  // namespace formatter
}  // namespace v1
}  // namespace blackhole
