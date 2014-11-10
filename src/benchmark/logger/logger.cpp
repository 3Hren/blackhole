#include <epicmeter/benchmark.hpp>

#include <blackhole/formatter/string.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/macro.hpp>
#include <blackhole/sink/null.hpp>
#include <blackhole/sink/stream.hpp>

#include "../util.hpp"

#define BH_BASE_LOG(__log__, ...) \
    if (auto record = (__log__).open_record()) \
        if (blackhole::aux::syntax_check(__VA_ARGS__)) \
            blackhole::aux::logger::make_pusher((__log__), record, __VA_ARGS__)

#define BENCHMARK_BASELINE_X(...) void TT_CONCATENATE(f, __LINE__)()
#define BENCHMARK_RELATIVE_X(...) void TT_CONCATENATE(f, __LINE__)()

namespace { enum level_t { debug, info }; }

#define MESSAGE_LONG "Something bad is going on but I can handle it"

static const std::string FORMAT_BASE    = "[%(timestamp)s]: %(message)s";
static const std::string FORMAT_VERBOSE = "[%(timestamp)s] [%(severity)s]: %(message)s";

BENCHMARK(LogStringToNull, Baseline) {
    static auto log = initialize<
        blackhole::verbose_logger_t<level_t>,
        blackhole::formatter::string_t,
        blackhole::sink::null_t
    >().formatter(FORMAT_VERBOSE).sink().log(level_t::debug).get();

    BH_LOG(log, level_t::info, MESSAGE_LONG)(
        "id", 42,
        "info", "le string"
    );
}

BENCHMARK_BASELINE(Logger, Base) {
    static auto log = initialize<
        blackhole::logger_base_t,
        blackhole::formatter::string_t,
        blackhole::sink::null_t
    >().formatter(FORMAT_BASE).sink().log().get();

    BH_BASE_LOG(log, MESSAGE_LONG)(
        "id", 42,
        "info", "le string"
    );
}

BENCHMARK(Logger, Verbose) {
    static auto log = initialize<
        blackhole::verbose_logger_t<level_t>,
        blackhole::formatter::string_t,
        blackhole::sink::null_t
    >().formatter(FORMAT_VERBOSE).sink().log(level_t::debug).get();

    BH_LOG(log, level_t::info, MESSAGE_LONG)(
        "id", 42,
        "info", "le string"
    );
}

BENCHMARK_BASELINE_X(Limits, Practical) {
    static const char MESSAGE[] = "[1412592701.561182] [0]: Something bad is going on but I can handle it";
    std::cout << MESSAGE << std::endl;
}

BENCHMARK_RELATIVE_X(Limits, Experimental) {
    static auto log = initialize<
        blackhole::verbose_logger_t<level_t>,
        blackhole::formatter::string_t,
        blackhole::sink::stream_t
    >()
        .formatter(FORMAT_VERBOSE)
        .sink(blackhole::sink::stream_t::output_t::stdout)
        .log(level_t::debug)
        .get();
    BH_LOG(log, level_t::info, MESSAGE_LONG);
}

BENCHMARK_BASELINE(Filtering, Rejected) {
    static auto log = initialize<
        blackhole::verbose_logger_t<level_t>,
        blackhole::formatter::string_t,
        blackhole::sink::null_t
    >()
        .formatter(FORMAT_BASE)
        .sink()
        .log(level_t::debug)
        .mod(std::bind(&filter_by::verbosity<level_t>, std::placeholders::_1, level_t::info))
        .get();

    BH_LOG(log, level_t::debug, MESSAGE_LONG)(
        "id", 42,
        "info", "le string"
    );
}

BENCHMARK(Filtering, Accepted) {
    static auto log = initialize<
        blackhole::verbose_logger_t<level_t>,
        blackhole::formatter::string_t,
        blackhole::sink::null_t
    >()
        .formatter(FORMAT_BASE)
        .sink()
        .log(level_t::debug)
        .mod(std::bind(&filter_by::verbosity<level_t>, std::placeholders::_1, level_t::info))
        .get();

    BH_LOG(log, level_t::info, MESSAGE_LONG)(
        "id", 42,
        "info", "le string"
    );
}
