#include <blackhole/log.hpp>
#include <blackhole/repository.hpp>

using namespace blackhole;

//! Our logger severity enumeration.
enum class level {
    debug,
    info,
    warning,
    error
};

namespace blackhole { namespace sink {

//! Priority mapping function overload for proper syslog mapping.
template<>
struct priority_traits<level> {
    static inline
    priority_t map(level lvl) {
        switch (lvl) {
        case level::debug:
            return priority_t::debug;
        case level::info:
            return priority_t::info;
        case level::warning:
            return priority_t::warning;
        case level::error:
            return priority_t::err;
        }

        return priority_t::debug;
    }
};

} } // namespace blackhole::sink

//! Attribute mapping from its real values to human-readabl string representation.
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
//! \brief Manually or from file - whatever. The main aim - is to get initialized `log_config_t` object.
//! For logstash we need log object which sends json-packed messages through tcp socket.
/*! Formatter config looks like:
 *  "formatter": {
 *      "json": {
 *          "newline": true,
 *          "mapping": {
 *              "naming": {
 *                  "message": "@message",
 *                  "timestamp": "@timestamp"
 *              },
 *              "positioning": {
 *                  "/": ["@message", "@timestamp"],
 *                  "/fields": "*"
 *              }
 *          }
 *      }
 *  }
 *
 *  Sink config can be:
 *  "sink": {
 *      "socket": {
 *          "type": "tcp",
 *          "host": "localhost",
 *          "port": 50030
 *      }
 *  }
 */
void init() {
    mapping::value_t mapper;
    mapper.add<level>("severity", &map_severity);
    mapper.add<std::time_t>("@timestamp", &map_timestamp);

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
                    { "/", std::vector<std::string> { "@message", "@timestamp" } },
                    { "/fields", std::string("*") }
                }
            }
        },
        mapper
    };

    sink_config_t sink = {
        "socket",
        std::map<std::string, std::string>{
            { "type", "tcp" },
            { "host", "localhost" },
            { "port", "50030" }
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
