/// Format string and write it to null.

#include <benchmark/benchmark.h>

#include <blackhole/logger.hpp>
#include <blackhole/wrapper.hpp>

namespace blackhole {
namespace benchmark {

using attribute::value_t;
using attribute::owned_t;

static
void
literal(::benchmark::State& state) {
    root_logger_t log({});

    while (state.KeepRunning()) {
        log.log(0, "[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326");
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
string(::benchmark::State& state) {
    root_logger_t log({});

    const std::string string("[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326");
    while (state.KeepRunning()) {
        log.log(0, string);
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_reject(::benchmark::State& state) {
    root_logger_t log({});
    log.filter([](const record_t&) -> bool {
        return false;
    });

    while (state.KeepRunning()) {
        log.log(0, "[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326");
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_arg(::benchmark::State& state) {
    root_logger_t log({});

    while (state.KeepRunning()) {
        log.log(0, "[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET {} HTTP/1.0' 200 2326",
            "/porn.png"
        );
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_args(::benchmark::State& state) {
    root_logger_t log({});

    while (state.KeepRunning()) {
        log.log(0, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
            "[::]",
            "esafronov",
            "10/Oct/2000:13:55:36 -0700",
            "/porn.png",
            200,
            2326
        );
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_args_using_cpp14_formatter(::benchmark::State& state) {
    root_logger_t log({});

    while (state.KeepRunning()) {
        constexpr auto formatter = blackhole::detail::formatter<
            blackhole::detail::literal_count("{} - {} [{}] 'GET {} HTTP/1.0' {} {}")
        >("{} - {} [{}] 'GET {} HTTP/1.0' {} {}");

        log.log(0, formatter,
            "[::]",
            "esafronov",
            "10/Oct/2000:13:55:36 -0700",
            "/porn.png",
            200,
            2326
        );
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_attributes(::benchmark::State& state) {
    root_logger_t log({});

    while (state.KeepRunning()) {
        log.log(0, {
            {"key#1", value_t(42)},
            {"key#2", value_t(3.1415)},
            {"key#3", value_t("value")}
        }, "[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326");
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_args_and_attributes(::benchmark::State& state) {
    root_logger_t log({});

    while (state.KeepRunning()) {
        log.log(0,
            {
                {"key#1", value_t(42)},
                {"key#2", value_t(3.1415)},
                {"key#3", value_t("value")}
            }, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
            "[::]",
            "esafronov",
            "10/Oct/2000:13:55:36 -0700",
            "/porn.png",
            200,
            2326
        );
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_args_and_attributes_and_wrapper(::benchmark::State& state) {
    root_logger_t log({});
    wrapper_t wrapper{log, {
        {"key#0", owned_t(500)},
        {"key#1", owned_t("value#1")}
    }};

    while (state.KeepRunning()) {
        wrapper.log(0,
            {
                {"key#1", value_t(42)},
                {"key#2", value_t(3.1415)},
                {"key#3", value_t("value")}
            }, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
            "[::]",
            "esafronov",
            "10/Oct/2000:13:55:36 -0700",
            "/porn.png",
            200,
            2326
        );
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_args_and_attributes_and_two_wrappers(::benchmark::State& state) {
    root_logger_t log({});

    wrapper_t wrapper1{log, {
        {"key#0", owned_t(500)},
        {"key#1", owned_t("value#1")}
    }};

    wrapper_t wrapper2{wrapper1, {
        {"key#2", owned_t(500)},
        {"key#3", owned_t("value#3")}
    }};

    while (state.KeepRunning()) {
        wrapper2.log(0,
            {
                {"key#1", value_t(42)},
                {"key#2", value_t(3.1415)},
                {"key#3", value_t("value")}
            }, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
            "[::]",
            "esafronov",
            "10/Oct/2000:13:55:36 -0700",
            "/porn.png",
            200,
            2326
        );
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_args_and_attributes_and_three_wrappers(::benchmark::State& state) {
    root_logger_t log({});

    wrapper_t wrapper1{log, {
        {"key#0", owned_t(500)},
        {"key#1", owned_t("value#1")}
    }};

    wrapper_t wrapper2{wrapper1, {
        {"key#2", owned_t(500)},
        {"key#3", owned_t("value#1")}
    }};

    wrapper_t wrapper3{wrapper2, {
        {"key#4", owned_t(500)},
        {"key#5", owned_t("value#5")}
    }};

    while (state.KeepRunning()) {
        wrapper3.log(0,
            {
                {"key#6", value_t(42)},
                {"key#7", value_t(3.1415)},
                {"key#8", value_t("value")}
            }, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
            "[::]",
            "esafronov",
            "10/Oct/2000:13:55:36 -0700",
            "/porn.png",
            200,
            2326
        );
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(literal);
BENCHMARK(literal_reject);
BENCHMARK(string);
BENCHMARK(literal_with_arg);
BENCHMARK(literal_with_args);
BENCHMARK(literal_with_args_using_cpp14_formatter);

BENCHMARK(literal_with_attributes);
BENCHMARK(literal_with_args_and_attributes);

BENCHMARK(literal_with_args_and_attributes_and_wrapper);
BENCHMARK(literal_with_args_and_attributes_and_two_wrappers);
BENCHMARK(literal_with_args_and_attributes_and_three_wrappers);

}  // namespace benchmark
}  // namespace blackhole
