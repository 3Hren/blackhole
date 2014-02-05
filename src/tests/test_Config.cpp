#include "Mocks.hpp"

TEST(FactoryTraits, StringFormatterConfig) {
    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    auto actual = factory_traits<formatter::string_t>::map_config(formatter.config);

    EXPECT_EQ("[%s]: %s", actual.pattern);
    EXPECT_EQ(std::vector<std::string>({ "timestamp", "message" }), actual.attribute_names);
}

TEST(FactoryTraits, JsonFormatterConfig) {
    formatter_config_t formatter("json");
    formatter["newline"] = true;
    formatter["mapping"]["message"] = "@message";
    formatter["routing"]["/"] = std::vector<std::string> { "message" };
    formatter["routing"]["/fields"] = "*";

    formatter::json::config_t actual = factory_traits<formatter::json_t>::map_config(formatter.config);

    using namespace formatter::json::map;
    EXPECT_TRUE(actual.newline);
    ASSERT_TRUE(actual.naming.find("message") != actual.naming.end());
    EXPECT_EQ("@message", actual.naming["message"]);

    typedef std::unordered_map<std::string, routing_t::routes_t> specified_positioning_t;
    ASSERT_TRUE(actual.routing.specified.find("message") != actual.routing.specified.end());
    EXPECT_EQ(std::vector<std::string>({}), actual.routing.specified["message"]);
    EXPECT_EQ(std::vector<std::string>({ "fields" }), actual.routing.unspecified);
}

TEST(FactoryTraits, FileSinkConfig) {
    sink_config_t sink("files");
    sink["path"] = "/dev/null";
    sink["autoflush"] = false;

    auto actual = factory_traits<sink::files_t<>>::map_config(sink.config);

    EXPECT_EQ("/dev/null", actual.path);
    EXPECT_FALSE(actual.autoflush);
}
