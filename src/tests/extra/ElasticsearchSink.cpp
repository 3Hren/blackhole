#include <blackhole/formatter/json.hpp>
#include <blackhole/sink/elasticsearch.hpp>

#include "../global.hpp"

using namespace blackhole;
using namespace elasticsearch;

TEST(elasticsearch_t, Class) {
    using blackhole::sink::elasticsearch_t;
    elasticsearch_t sink;
    UNUSED(sink);
}

TEST(elasticsearch_t, ConfigConstructor) {
    sink::elasticsearch_t::config_type config;
    sink::elasticsearch_t sink(config);
    UNUSED(sink);
}

TEST(utils, MakePath) {
    EXPECT_EQ("/", elasticsearch::utils::make_path("/"));
    EXPECT_EQ("/index", elasticsearch::utils::make_path("index"));
    EXPECT_EQ("/index/type", elasticsearch::utils::make_path("index", "type"));
}

TEST(nodes_info_t, Class) {
    actions::nodes_info_t action;
    UNUSED(action);
}

TEST(nodes_info_t, Method) {
    EXPECT_EQ(request::method_t::get, actions::nodes_info_t::method::value);
}

TEST(nodes_info_t, PathByDefault) {
    actions::nodes_info_t action;
    EXPECT_EQ("/_nodes/_all/none", action.path());
}

TEST(bulk_write_t, Class) {
    actions::bulk_write_t action("index", "type", std::vector<std::string> { "{}" });
    UNUSED(action);
}

TEST(bulk_write_t, Method) {
    EXPECT_EQ(request::method_t::post, actions::bulk_write_t::method::value);
}

TEST(bulk_write_t, Path) {
    actions::bulk_write_t action("index", "type", std::vector<std::string> { "{}" });
    EXPECT_EQ("/index/type/_bulk", action.path());
}

TEST(elasticsearch_t, ManualLocal) {
    using blackhole::sink::elasticsearch_t;
    elasticsearch_t sink;
    std::string msg = "{'@message':'le message', '@timestamp':'2014-06-02T21:20:00.000Z'}";
    boost::algorithm::replace_all(msg, "'", "\"");
    for (int i = 0; i < 200; ++i) {
        sink.consume(msg);
    }
}
