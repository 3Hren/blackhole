#include <blackhole/formatter/json.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/repository/config/formatter.hpp>
#include <blackhole/repository/config/sink.hpp>
#include <blackhole/repository/factory/traits.hpp>
#include <blackhole/sink/files.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(FactoryTraits, StringFormatterConfig) {
    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    auto actual = aux::config_mapper<formatter::string_t>::map(formatter.config());

    EXPECT_EQ("[%(timestamp)s]: %(message)s", actual.pattern);
}

TEST(FactoryTraits, JsonFormatterConfig) {
    formatter_config_t formatter("json");
    formatter["newline"] = true;
    formatter["mapping"]["message"] = "@message";
    formatter["routing"]["/"] = dynamic_t::array_t { "message" };
    formatter["routing"]["/fields"] = "*";

    auto actual = aux::config_mapper<formatter::json_t>::map(formatter.config());

    using namespace formatter::json::map;
    EXPECT_TRUE(actual.newline);
    ASSERT_TRUE(actual.naming.find("message") != actual.naming.end());
    EXPECT_EQ("@message", actual.naming["message"]);

    ASSERT_TRUE(actual.routing.specified.find("message") != actual.routing.specified.end());
    EXPECT_EQ(std::vector<std::string>({}), actual.routing.specified["message"]);
    EXPECT_EQ(std::vector<std::string>({ "fields" }), actual.routing.unspecified);
}

TEST(FactoryTraits, FileSinkConfig) {
    sink_config_t sink("files");
    sink["path"] = "/dev/null";
    sink["autoflush"] = false;

    auto actual = aux::config_mapper<sink::files_t<>>::map(sink.config());

    EXPECT_EQ("/dev/null", actual.path);
    EXPECT_FALSE(actual.autoflush);
}
