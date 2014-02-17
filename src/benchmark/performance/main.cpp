#include <memory>

#include "celero/Celero.h"

#include <blackhole/log.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/sink/null.hpp>
#include <blackhole/repository.hpp>

using namespace blackhole;

enum level {
    debug,
    info,
    warning,
    error,
    critical
};

const int N = 200000;

std::string map_timestamp(const timeval& tv) {
    char str[64];

    struct tm tm;
    localtime_r((time_t *)&tv.tv_sec, &tm);
    if (std::strftime(str, sizeof(str), "%F %T", &tm)) {
        return str;
    }

    return "UNKNOWN";
}

std::string map_severity(const level& level) {
    static const char* descriptions[] = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "CRITICAL"
    };

    if (static_cast<std::size_t>(level) < sizeof(descriptions) / sizeof(*descriptions))
        return descriptions[level];
    return std::to_string(static_cast<int>(level));
}

void init_blackhole_log() {
    repository_t<level>::instance().configure<
        sink::null_t,
        formatter::string_t
    >();

    mapping::value_t mapper;
    mapper.add<timeval>("timestamp", &map_timestamp);
    mapper.add<level>("severity", &map_severity);

    formatter_config_t formatter("string", mapper);
    formatter["pattern"] = "[%(timestamp)s] [%(severity)s]: %(message)s";

    sink_config_t sink("null");

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "root", { frontend } };

    repository_t<level>::instance().add_config(config);
}

verbose_logger_t<level> *log_;

int main(int argc, char** argv) {
    init_blackhole_log();
    auto log = repository_t<level>::instance().root();
    log_ = &log;

    celero::Run(argc, argv);
    return 0;
}


BASELINE(CeleroBenchTest, BoostLog, 0, N) {
    BH_LOG((*log_), warning, "Something bad is going on but I can handle it");
}
