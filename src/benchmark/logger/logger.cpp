#include <ticktack/benchmark.hpp>

#include <blackhole/formatter/string.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/macro.hpp>
#include <blackhole/sink/null.hpp>
#include <blackhole/utils/unique.hpp>

namespace { enum level_t { info }; }

namespace {

blackhole::verbose_logger_t<level_t> initialize() {
    auto formatter = blackhole::utils::make_unique<
        blackhole::formatter::string_t
    >("%(message)s");

    auto sink = blackhole::utils::make_unique<
        blackhole::sink::null_t
    >();

    auto frontend = blackhole::utils::make_unique<
        blackhole::frontend_t<
            blackhole::formatter::string_t,
            blackhole::sink::null_t
        >
    >(std::move(formatter), std::move(sink));

    blackhole::verbose_logger_t<level_t> log;
    log.add_frontend(std::move(frontend));
    return log;
}

static const char MESSAGE_LONG[] = "Something bad is going on but I can handle it";

} // namespace

BENCHMARK(LogStringToNull, Baseline) {
    static auto log = initialize();

    BH_LOG(log, level_t::info, MESSAGE_LONG)(
        "answer", 42,
        "string", "le string"
    );
}
