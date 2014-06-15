#include <blackhole/sink/elasticsearch/request/utils.hpp>

#include "../../global.hpp"

TEST(utils, MakePath) {
    EXPECT_EQ("/", elasticsearch::utils::make_path("/"));
    EXPECT_EQ("/index", elasticsearch::utils::make_path("index"));
    EXPECT_EQ("/index/type", elasticsearch::utils::make_path("index", "type"));
}
