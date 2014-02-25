#include <memory>

#include <boost/lexical_cast.hpp>

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
    return boost::lexical_cast<std::string>(level);
}

formatter::string_t fmt("[%(timestamp)s] [%(severity)s]: %(message)s");

void init_blackhole_log() {
    repository_t<level>::instance().configure<
        sink::null_t,
        formatter::string_t
    >();

    mapping::value_t mapper;
    mapper.add<timeval>("timestamp", &map_timestamp);
    mapper.add<level>("severity", &map_severity);
    fmt.set_mapper(mapper);

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

const int N = 10000;
BASELINE(PureStringFormatter, Baseline, 0, N) {
    log::record_t record;
    record.attributes.insert(keyword::message() = "Something bad is going on but I can handle it");

    timeval tv;
    gettimeofday(&tv, nullptr);
    record.attributes.insert(keyword::timestamp() = tv);
    record.attributes.insert(keyword::severity<level>() = level::warning);
    celero::DoNotOptimizeAway(fmt.format(record));
}

#include <blackhole/detail/datetime.hpp>

const int N2 = 100000;
BASELINE(DatetimeGenerator, Baseline, 0, N2) {
    std::time_t time = std::time(nullptr);
    std::tm tm;
    localtime_r(&time, &tm);
    char buf[64];
    strftime(buf, 64, "%Y-%m-%d %H:%M:%S", &tm);
    celero::DoNotOptimizeAway(buf);
}

BASELINE(DatetimeGeneratorUsingLocale, Baseline, 0, N2) {
    std::time_t time = std::time(nullptr);
    std::tm tm;
    localtime_r(&time, &tm);
    char buf[64];
    strftime(buf, 64, "%c", &tm);
    celero::DoNotOptimizeAway(buf);
}

aux::datetime::generator_t generator2 = aux::datetime::generator_factory_t::make("%c");

BENCHMARK(DatetimeGenerator, Generator, 0, N2) {
    static std::string str;
    static aux::datetime::generator_t generator(aux::datetime::generator_factory_t::make("%Y-%m-%d %H:%M:%S"));
    static aux::attachable_ostringstream stream(str);

    std::time_t time = std::time(nullptr);
    std::tm tm;
    localtime_r(&time, &tm);
    generator(stream, tm);
    celero::DoNotOptimizeAway(str);
    str.clear();
}

BENCHMARK(DatetimeGeneratorUsingLocale, Generator, 0, N2) {
    static std::string str;
    static aux::datetime::generator_t generator(aux::datetime::generator_factory_t::make("%c"));
    static aux::attachable_ostringstream stream(str);

    std::time_t time = std::time(nullptr);
    std::tm tm;
    localtime_r(&time, &tm);
    generator(stream, tm);
    celero::DoNotOptimizeAway(str);
    str.clear();
}
