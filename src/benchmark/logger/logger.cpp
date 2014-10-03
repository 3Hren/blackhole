#include <ticktack/benchmark.hpp>

#include <blackhole/detail/util/unique.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/macro.hpp>
#include <blackhole/sink/null.hpp>

namespace { enum level_t { info }; }

namespace {

template<class Log>
Log
initialize(const std::string& format = "[%(timestamp)s] [%(severity)s]: %(message)s") {
    auto formatter = blackhole::aux::util::make_unique<
        blackhole::formatter::string_t
    >(format);

    auto sink = blackhole::aux::util::make_unique<
        blackhole::sink::null_t
    >();

    auto frontend = blackhole::aux::util::make_unique<
        blackhole::frontend_t<
            blackhole::formatter::string_t,
            blackhole::sink::null_t
        >
    >(std::move(formatter), std::move(sink));

    Log log;
    log.add_frontend(std::move(frontend));
    return log;
}

static const char MESSAGE_LONG[] = "Something bad is going on but I can handle it";

} // namespace

#define BH_BASE_LOG(__log__, ...) \
    if (auto record = (__log__).open_record()) \
        if (blackhole::aux::syntax_check(__VA_ARGS__)) \
            blackhole::aux::logger::make_pusher((__log__), record, __VA_ARGS__)

BENCHMARK(LogStringToNull, Baseline) {
    static auto log = initialize<blackhole::verbose_logger_t<level_t>>();

    BH_LOG(log, level_t::info, MESSAGE_LONG)(
        "answer", 42,
        "string", "le string"
    );
}

BENCHMARK_BASELINE(Logger, Base) {
    static auto log = initialize<blackhole::logger_base_t>("[%(timestamp)s]: %(message)s");

    BH_BASE_LOG(log, MESSAGE_LONG)(
        "answer", 42,
        "string", "le string"
    );
}

BENCHMARK_RELATIVE(Logger, Verbose) {
    static auto log = initialize<blackhole::verbose_logger_t<level_t>>("[%(timestamp)s]: %(message)s");

    BH_LOG(log, level_t::info, MESSAGE_LONG)(
        "answer", 42,
        "string", "le string"
    );
}
