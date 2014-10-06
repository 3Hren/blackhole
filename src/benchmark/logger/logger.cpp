#include <ticktack/benchmark.hpp>

#include <blackhole/detail/util/unique.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/macro.hpp>
#include <blackhole/sink/null.hpp>
#include <blackhole/sink/stream.hpp>

#define BENCHMARK_BASELINE_X(...) void TT_CONCATENATE(f, __LINE__)()
#define BENCHMARK_RELATIVE_X(...) void TT_CONCATENATE(f, __LINE__)()

namespace { enum level_t { info }; }

namespace {

static const char FORMAT[] = "[%(timestamp)s]: %(message)s";
static const char DEFAULT_FORMAT[] = "[%(timestamp)s] [%(severity)s]: %(message)s";
static const char MESSAGE_LONG[] = "Something bad is going on but I can handle it";

template<class Log>
Log
initialize(const std::string& format = DEFAULT_FORMAT) {
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

template<class L, class F, class S>
struct initializer_t {
    std::unique_ptr<F> f;

    initializer_t(std::unique_ptr<F> f) :
        f(std::move(f))
    {}

    initializer_t(initializer_t&& other) :
        f(std::move(other.f))
    {}

    template<class... Args>
    L operator()(Args&&... args) {
        auto sink = blackhole::aux::util::make_unique<
            S
        >(std::forward<Args>(args)...);

        auto frontend = blackhole::aux::util::make_unique<
            blackhole::frontend_t<F, S>
        >(std::move(f), std::move(sink));

        L log;
        log.add_frontend(std::move(frontend));
        return log;
    }
};

template<class L, class F, class S, class... Args>
initializer_t<L, F, S>
initialize(Args&&... args) {
    auto formatter = blackhole::aux::util::make_unique<
        F
    >(std::forward<Args>(args)...);
    return initializer_t<L, F, S>(std::move(formatter));
}

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
    static auto log = initialize<blackhole::logger_base_t>(FORMAT);

    BH_BASE_LOG(log, MESSAGE_LONG)(
        "answer", 42,
        "string", "le string"
    );
}

BENCHMARK_RELATIVE(Logger, Verbose) {
    static auto log = initialize<blackhole::verbose_logger_t<level_t>>(FORMAT);

    BH_LOG(log, level_t::info, MESSAGE_LONG)(
        "answer", 42,
        "string", "le string"
    );
}

BENCHMARK_BASELINE_X(Limits, Practical) {
    static const char MESSAGE[] = "[1412592701.561182]: Something bad is going on but I can handle it";
    std::cout << MESSAGE << std::endl;
}

BENCHMARK_RELATIVE_X(Limits, Experimental) {
    static auto log = initialize<
        blackhole::verbose_logger_t<level_t>,
        blackhole::formatter::string_t,
        blackhole::sink::stream_t
    >(FORMAT)(blackhole::sink::stream_t::output_t::stdout);
    BH_LOG(log, level_t::info, MESSAGE_LONG);
}
