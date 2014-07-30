#include <memory>

#include <boost/lexical_cast.hpp>

#include "celero/Celero.h"

#include <blackhole/logger.hpp>
#include <blackhole/macro.hpp>
#include <blackhole/sink/null.hpp>
#include <blackhole/repository.hpp>

#define FIRE_DATETIME_GENERATOR
#define FIRE_STRING_FORMATTER
#define FIRE_STRING_TO_NULL_LOGGING

const char MESSAGE_LONG[] = "Something bad is going on but I can handle it";

using namespace blackhole;

#ifdef FIRE_STRING_FORMATTER
enum level {
    debug,
    info,
    warning,
    error,
    critical
};

void map_timestamp(blackhole::aux::attachable_ostringstream& stream, const timeval& tv) {
    char str[64];

    struct tm tm;
    localtime_r((time_t *)&tv.tv_sec, &tm);
    if (std::strftime(str, sizeof(str), "%F %T", &tm)) {
        stream << str;
    } else {
        stream << "UNKNOWN";
    }
}

void map_severity(blackhole::aux::attachable_ostringstream& stream, const level& level) {
    static const char* descriptions[] = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "CRITICAL"
    };

    if (static_cast<std::size_t>(level) < sizeof(descriptions) / sizeof(*descriptions))
        stream << descriptions[level];
    else
        stream << level;
}

formatter::string_t fmt("[%(timestamp)s] [%(severity)s]: %(message)s");

void initialize() {
    mapping::value_t mapper;
    mapper.add<timeval>("timestamp", &map_timestamp);
    mapper.add<level>("severity", &map_severity);
    fmt.set_mapper(mapper);
}
#endif

int main(int argc, char** argv) {
#ifdef FIRE_STRING_FORMATTER
    initialize();
#endif
    celero::Run(argc, argv);
    return 0;
}

#ifdef FIRE_STRING_FORMATTER
static const int STRING_FORMATTER_SAMPLES = 30;
static const int STRING_FORMATTER_CALLS = 100000;
BASELINE(PureStringFormatter, Baseline, STRING_FORMATTER_SAMPLES, STRING_FORMATTER_CALLS) {
    log::record_t record;
    record.attributes.insert(keyword::message() = "Something bad is going on but I can handle it");

    timeval tv;
    gettimeofday(&tv, nullptr);
    record.attributes.insert(keyword::timestamp() = tv);
    record.attributes.insert(keyword::severity<level>() = level::warning);
    celero::DoNotOptimizeAway(fmt.format(record));
}
#endif

#ifdef FIRE_DATETIME_GENERATOR
#include <blackhole/detail/datetime.hpp>

static const int DATETIME_GENERATOR_SAMPLES = 30;
static const int DATETIME_GENERATOR_CALLS = 100000;
BASELINE(DatetimeGenerator, Baseline, DATETIME_GENERATOR_SAMPLES, DATETIME_GENERATOR_CALLS) {
    std::time_t time = std::time(nullptr);
    std::tm tm;
    localtime_r(&time, &tm);
    char buf[64];
    strftime(buf, 64, "%Y-%m-%d %H:%M:%S", &tm);
    celero::DoNotOptimizeAway(buf);
}

BENCHMARK(DatetimeGenerator, Generator, DATETIME_GENERATOR_SAMPLES, DATETIME_GENERATOR_CALLS) {
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

BASELINE(DatetimeGeneratorUsingLocale, Baseline, DATETIME_GENERATOR_SAMPLES, DATETIME_GENERATOR_CALLS) {
    std::time_t time = std::time(nullptr);
    std::tm tm;
    localtime_r(&time, &tm);
    char buf[64];
    strftime(buf, 64, "%c", &tm);
    celero::DoNotOptimizeAway(buf);
}

BENCHMARK(DatetimeGeneratorUsingLocale, Generator, DATETIME_GENERATOR_SAMPLES, DATETIME_GENERATOR_CALLS) {
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
#endif

#ifdef FIRE_STRING_TO_NULL_LOGGING
#include <blackhole/sink/null.hpp>

static const int LOG_STRING_TO_NULL_SAMPLES = 30;
static const int LOG_STRING_TO_NULL_CALLS = 200000;

namespace log_string_to_null {

enum level {
    debug
};

verbose_logger_t<level> init_log() {
    auto formatter = utils::make_unique<
        formatter::string_t
    >("%(message)s");

    auto sink = utils::make_unique<
        sink::null_t
    >();

    auto frontend = utils::make_unique<
        frontend_t<
            formatter::string_t,
            sink::null_t
        >
    >(std::move(formatter), std::move(sink));

    verbose_logger_t<level> log;
    log.add_frontend(std::move(frontend));
    return log;
}

} // namespace log_string_to_null

BASELINE(LogStringToNull, Baseline, LOG_STRING_TO_NULL_SAMPLES, LOG_STRING_TO_NULL_CALLS) {
    static verbose_logger_t<log_string_to_null::level> log =
        log_string_to_null::init_log();

    BH_LOG(log, log_string_to_null::debug, MESSAGE_LONG)(attribute::list({
        {"int", 42},
        {"string", "le string"},
    }));
}
#endif
