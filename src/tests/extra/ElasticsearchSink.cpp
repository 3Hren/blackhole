#include <blackhole/sink/elasticsearch.hpp>

#include "../global.hpp"

using namespace blackhole;
using blackhole::sink::elasticsearch_t;

TEST(elasticsearch_t, Class) {
    elasticsearch_t sink;
    UNUSED(sink);
}

TEST(elasticsearch_t, Manual) {
    elasticsearch_t sink;
    for (int i = 0; i < 200; ++i) {
        sink.consume((boost::format("{\"key\":%d}") % i).str());
    }
}
