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

TEST(elasticsearch_t, IsThreadSafe) {
    static_assert(
        sink::thread_safety<
            sink::elasticsearch_t
        >::type::value == sink::thread::safety_t::safe,
        "`elasticsearch_t` sink must be thread safe"
    );
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
