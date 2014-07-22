#include <blackhole/repository.hpp>
#include <blackhole/sink/elasticsearch.hpp>

#include "../../global.hpp"

using namespace blackhole;

TEST(Factory, ElasticsearchJsonFrontend) {
    external_factory_t factory;
    factory.add<sink::elasticsearch_t, formatter::string_t>();

    formatter_config_t formatter("string");
    formatter["pattern"] = "%(message)s";

    sink_config_t sink("elasticsearch");
    sink["bulk"] = 100;
    sink["interval"] = 1000;
    sink["workers"] = 1;
    sink["index"] = "logs";
    sink["type"] = "log";
    sink["endpoints"] = dynamic_t::array_t({"localhost:9200"});
    sink["sniffer"]["when"]["start"] = true;
    sink["sniffer"]["when"]["error"] = true;
    sink["sniffer"]["interval"] = 60000;
    sink["connections"] = 20;
    sink["retries"] = 3;
    sink["timeout"] = 1000;

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}
