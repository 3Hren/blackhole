#include "Mocks.hpp"

TEST(FactoryTraits, StringFormatterConfig) {
    formatter_config_t config = {
        "string",
        std::string("[%(timestamp)s]: %(message)s")
    };

    auto actual = factory_traits<formatter::string_t>::map_config(config.config);

    EXPECT_EQ("[%s]: %s", actual.pattern);
    EXPECT_EQ(std::vector<std::string>({ "timestamp", "message" }), actual.attribute_names);
}

TEST(FactoryTraits, JsonFormatterConfig) {
    formatter_config_t config = {
        "json",
        boost::any {
            std::vector<boost::any> {
                true,
                std::unordered_map<std::string, std::string> { { "message", "@message" } },
                std::unordered_map<std::string, boost::any> {
                    { "/", std::vector<std::string> { "message" } },
                    { "/fields", std::string("*") }
                }
            }
        }
    };

    formatter::json::config_t actual = factory_traits<formatter::json_t>::map_config(config.config);

    using namespace formatter::json::map;
    EXPECT_TRUE(actual.newline);
    ASSERT_TRUE(actual.naming.find("message") != actual.naming.end());
    EXPECT_EQ("@message", actual.naming["message"]);

    typedef std::unordered_map<std::string, positioning_t::positions_t> specified_positioning_t;
    ASSERT_TRUE(actual.positioning.specified.find("message") != actual.positioning.specified.end());
    EXPECT_EQ(std::vector<std::string>({}), actual.positioning.specified["message"]);
    EXPECT_EQ(std::vector<std::string>({ "fields" }), actual.positioning.unspecified);
}

TEST(FactoryTraits, FileSinkConfig) {
    sink_config_t config = {
        "files",
        std::map<std::string, boost::any> {
            { "path", std::string("/dev/null") },
            { "autoflush", false }
        }
    };

    auto actual = factory_traits<sink::file_t<>>::map_config(config.config);

    EXPECT_EQ("/dev/null", actual.path);
    EXPECT_FALSE(actual.autoflush);
}
