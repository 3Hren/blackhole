#include <blackhole/log.hpp>
#include <blackhole/repository.hpp>

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
std::string map_severity(level lvl) {
    static std::string LEVEL[] = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR"
    };

    auto value = static_cast<aux::underlying_type<level>::type>(lvl);
    if (value >= 0 && value < sizeof(LEVEL) / sizeof(LEVEL[0])) {
        return LEVEL[value];
    }

    return "UNKNOWN";
}

std::string map_timestamp(const std::time_t& time) {
    char mbstr[128];
    if(std::strftime(mbstr, 128, "%F %T", std::gmtime(&time))) {
        return std::string(mbstr);
    }
    return std::string("?");
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
                "naming": {
                    "message": "@message",
                    "timestamp": "@timestamp"
                },
                "positioning": {
                    "/": ["@message", "@timestamp"],
                    "/fields": "*"
                }
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
    repository_t<level>::instance().configure<
        sink::socket_t<boost::asio::ip::tcp>, formatter::json_t
    >();

    mapping::value_t mapper;
    mapper.add<keyword::tag::severity_t<level>>(&map_severity);
    mapper.add<keyword::tag::timestamp_t>(&map_timestamp);

    formatter_config_t formatter = {
        "json",
        boost::any {
            std::vector<boost::any> {
                true,
                std::unordered_map<std::string, std::string> {
                    { "message", "@message" },
                    { "timestamp", "@timestamp" }
                },
                std::unordered_map<std::string, boost::any> {
                    { "/", std::vector<std::string> { "message", "timestamp" } },
                    { "/fields", std::string("*") }
                }
            }
        },
        mapper
    };

    sink_config_t sink = {
        "tcp",
        std::map<std::string, boost::any> {
            { "host", std::string("localhost") },
            { "port", std::uint16_t(50030) }
        }
    };

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "root", { frontend } };

    repository_t<level>::instance().init(config);
}

int main(int, char**) {
    init();
    verbose_logger_t<level> log = repository_t<level>::instance().root();

    BH_LOG(log, level::debug,   "[%d] %s - done", 0, "debug");
    BH_LOG(log, level::info,    "[%d] %s - done", 1, "info");
    BH_LOG(log, level::warning, "[%d] %s - done", 2, "warning");
    BH_LOG(log, level::error,   "[%d] %s - done", 3, "error");

    return 0;
}
