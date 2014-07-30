#include <blackhole/blackhole.hpp>

#include <blackhole/formatter/json.hpp>
#include <blackhole/sink/elasticsearch.hpp>

using namespace blackhole;

//! Our logger severity enumeration.
enum class level {
    debug,
    info,
    warning,
    error
};

namespace {

//! Attribute mapping from its real values to human-readable string representation.
void map_severity(blackhole::aux::attachable_ostringstream& stream, level lvl) {
    static std::string LEVEL[] = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR"
    };

    auto value = static_cast<aux::underlying_type<level>::type>(lvl);
    if (value >= 0 && value < sizeof(LEVEL) / sizeof(LEVEL[0])) {
        stream << LEVEL[value];
    } else {
        stream << "UNKNOWN";
    }
}

} // namespace

//! Initialization stage.
void init() {
    //! Register required frontend.
    repository_t::instance().configure<
        sink::elasticsearch_t,
        formatter::json_t
    >();

    mapping::value_t mapper;
    mapper.add<keyword::tag::severity_t<level>>(&map_severity);
    mapper.add<keyword::tag::timestamp_t>("%FT%T.%f");

    formatter_config_t formatter("json", mapper);
    formatter["newline"] = false;
    formatter["mapping"]["message"] = "@message";
    formatter["mapping"]["timestamp"] = "@timestamp";
    formatter["routing"]["/"] = dynamic_t::array_t { "message", "timestamp" };
    formatter["routing"]["/fields"] = "*";

    sink_config_t sink("elasticsearch");
    sink["bulk"] = 100;
    sink["interval"] = 1000;
    sink["workers"] = 1;
    sink["index"] = "logs";
    sink["type"] = "log";
    sink["endpoints"] = dynamic_t::array_t { "localhost:9200" };
    sink["sniffer"]["when"]["start"] = true;
    sink["sniffer"]["when"]["error"] = true;
    sink["sniffer"]["interval"] = 60000;
    sink["connections"] = 20;
    sink["retries"] = 3;
    sink["timeout"] = 1000;

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "root", { frontend } };

    repository_t::instance().add_config(config);
}

int main(int, char**) {
    init();
    auto log = repository_t::instance().root<level>();

    for (int id = 0; id < 200; ++id) {
        BH_LOG(log, level::debug, "this is a debug message")(attribute::list({
            {"answer", 42},
            {"id", id}
        }));
    }

    return 0;
}
