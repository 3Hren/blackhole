#include <blackhole/blackhole.hpp>
#include <blackhole/formatter/json.hpp>
#include <blackhole/sink/socket.hpp>

using namespace blackhole;

//! Our logger severity enumeration.
enum class level {
    debug,
    info,
    warning,
    error
};

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

void map_timestamp(blackhole::aux::attachable_ostringstream& stream, const timeval& tv) {
    char str[64];

    struct tm tm;
    localtime_r((time_t *)&tv.tv_sec, &tm);
    if (std::strftime(str, sizeof(str), "%F %T", &tm)) {
        char usecs[64];
        snprintf(usecs, sizeof(usecs), ".%06ld", (long)tv.tv_usec);
        stream << str << usecs;
    } else {
        stream << "UNKNOWN";
    }
}

//! Initialization stage.
//! \brief Manually or from file - whatever.
//! The main aim - is to get initialized `log_config_t` object.
//! For logstash we need log object which sends json-packed messages through tcp socket.
/*! Formatter config looks like:
    "formatter": {
        "json": {
            "newline": true,
            "mapping": {
                "message": "@message",
                "timestamp": "@timestamp"
            },
            "routing": {
                "/": ["@message", "@timestamp"],
                "/fields": "*"
            }
        }
    }

    Sink config can be:
    "sink": {
        "tcp": {
            "host": "localhost",
            "port": 50030
        }
    }
*/
void init() {
    //! Register required frontend.
    repository_t::instance().configure<
        sink::socket_t<boost::asio::ip::tcp>,
        formatter::json_t
    >();

    mapping::value_t mapper;
    mapper.add<keyword::tag::severity_t<level>>(&map_severity);
    mapper.add<keyword::tag::timestamp_t>(&map_timestamp);

    formatter_config_t formatter("json", mapper);
    formatter["newline"] = true;
    formatter["mapping"]["message"] = "@message";
    formatter["mapping"]["timestamp"] = "@timestamp";
    formatter["routing"]["/"] = dynamic_t::array_t { "message", "timestamp" };
    formatter["routing"]["/fields"] = "*";

    sink_config_t sink("tcp");
    sink["host"] = "localhost";
    sink["port"] = 50030;

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "root", { frontend } };

    repository_t::instance().add_config(config);
}

int main(int, char**) {
    init();
    verbose_logger_t<level> log = repository_t::instance().root<level>();

    BH_LOG(log, level::debug,   "[%d] %s - done", 0, "debug");
    BH_LOG(log, level::info,    "[%d] %s - done", 1, "info");
    BH_LOG(log, level::warning, "[%d] %s - done", 2, "warning");
    BH_LOG(log, level::error,   "[%d] %s - done", 3, "error");

    return 0;
}
