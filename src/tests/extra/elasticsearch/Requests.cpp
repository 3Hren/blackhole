#include <blackhole/sink/elasticsearch/request/bulk_write.hpp>
#include <blackhole/sink/elasticsearch/request/info.hpp>

#include "../../global.hpp"

using namespace elasticsearch;

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
