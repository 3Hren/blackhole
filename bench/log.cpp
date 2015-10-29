/// Format string and write it to null.

#include <benchmark/benchmark.h>

#include <blackhole/log.hpp>

namespace blackhole {
namespace benchmark {

static
void
literal(::benchmark::State& state) {
    log_t log;

    while (state.KeepRunning()) {
        log.info("[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326");
    }
}

static
void
string(::benchmark::State& state) {
    log_t log;

    const std::string string("[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326");
    while (state.KeepRunning()) {
        log.info(string);
    }
}

static
void
literal_with_arg(::benchmark::State& state) {
    log_t log;

    while (state.KeepRunning()) {
        log.info("[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET {} HTTP/1.0' 200 2326",
            "/porn.png");
    }
}

static
void
literal_with_args(::benchmark::State& state) {
    log_t log;

    while (state.KeepRunning()) {
        log.info("{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
            "[::]",
            "esafronov",
            "10/Oct/2000:13:55:36 -0700",
            "/porn.png",
            200,
            2326
        );
    }
}

static
void
cpp14_formatter_with_args(::benchmark::State& state) {
    log_t log;

    while (state.KeepRunning()) {
        constexpr auto formatter = blackhole::detail::formatter<
            blackhole::detail::literal_count("{} - {} [{}] 'GET {} HTTP/1.0' {} {}")
        >("{} - {} [{}] 'GET {} HTTP/1.0' {} {}");

        log.info(formatter,
            "[::]",
            "esafronov",
            "10/Oct/2000:13:55:36 -0700",
            "/porn.png",
            200,
            2326
        );
    }
}

static
void
literal_with_attributes(::benchmark::State& state) {
    log_t log;

    while (state.KeepRunning()) {
        log.info("[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326", {
            {"key#1", attribute_value_t(42)},
            {"key#2", attribute_value_t(3.1415)},
            {"key#3", attribute_value_t("value")}
        });
    }
}

static
void
literal_with_args_and_attributes(::benchmark::State& state) {
    log_t log;

    while (state.KeepRunning()) {
        log.info("{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
            {
                {"key#1", attribute_value_t(42)},
                {"key#2", attribute_value_t(3.1415)},
                {"key#3", attribute_value_t("value")}
            },
            "[::]",
            "esafronov",
            "10/Oct/2000:13:55:36 -0700",
            "/porn.png",
            200,
            2326
        );
    }
}

static
void
literal_with_args_and_attributes_and_wrapper(::benchmark::State& state) {
    log_t log;
    wrapper_t wrapper{log, {
        {"key#0", owned_attribute_value_t(500)},
        {"key#1", owned_attribute_value_t("value#1")}
    }};

    while (state.KeepRunning()) {
        wrapper.info("{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
            {
                {"key#1", attribute_value_t(42)},
                {"key#2", attribute_value_t(3.1415)},
                {"key#3", attribute_value_t("value")}
            },
            "[::]",
            "esafronov",
            "10/Oct/2000:13:55:36 -0700",
            "/porn.png",
            200,
            2326
        );
    }
}

BENCHMARK(literal);
BENCHMARK(string);
BENCHMARK(literal_with_arg);
BENCHMARK(literal_with_args);
BENCHMARK(cpp14_formatter_with_args);

BENCHMARK(literal_with_attributes);
BENCHMARK(literal_with_args_and_attributes);

BENCHMARK(literal_with_args_and_attributes_and_wrapper);

}  // namespace benchmark
}  // namespace blackhole
