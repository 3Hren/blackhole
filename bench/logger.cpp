/// Format string and write it to null.
#include <thread>

#include <benchmark/benchmark.h>

#include <blackhole/extensions/format.hpp>
#include <blackhole/handler.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/root.hpp>
#include <blackhole/wrapper.hpp>

namespace blackhole {
namespace benchmark {

using attribute::value_t;
using attribute::view_t;

static
void
literal(::benchmark::State& state) {
    root_logger_t root({});
    logger_facade<root_logger_t> logger(root);

    while (state.KeepRunning()) {
        logger.log(0, "[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326");
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
string(::benchmark::State& state) {
    root_logger_t root({});
    logger_facade<root_logger_t> logger(root);

    const std::string string("[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326");
    while (state.KeepRunning()) {
        logger.log(0, string);
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_reject(::benchmark::State& state) {
    root_logger_t root({});
    root.filter([](const record_t&) -> bool {
        return false;
    });

    logger_facade<root_logger_t> logger(root);

    while (state.KeepRunning()) {
        logger.log(0, "[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326");
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_arg(::benchmark::State& state) {
    root_logger_t root({});
    logger_facade<root_logger_t> logger(root);

    while (state.KeepRunning()) {
        logger.log(0, "[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET {} HTTP/1.0' 200 2326",
            "/porn.png"
        );
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_args(::benchmark::State& state) {
    root_logger_t root({});
    logger_facade<root_logger_t> logger(root);

    while (state.KeepRunning()) {
        logger.log(0, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
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

#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304
static
void
literal_with_args_using_cpp14_formatter(::benchmark::State& state) {
    root_logger_t root({});
    logger_facade<root_logger_t> logger(root);

    while (state.KeepRunning()) {
        constexpr auto formatter = blackhole::detail::formatter<
            blackhole::detail::literal_count("{} - {} [{}] 'GET {} HTTP/1.0' {} {}")
        >("{} - {} [{}] 'GET {} HTTP/1.0' {} {}");

        logger.log(0, formatter,
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
#endif

static
void
literal_with_attributes(::benchmark::State& state) {
    root_logger_t root({});
    logger_facade<root_logger_t> logger(root);

    while (state.KeepRunning()) {
        logger.log(0, {
            {"key#1", view_t(42)},
            {"key#2", view_t(3.1415)},
            {"key#3", view_t("value")}
        }, "[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326");
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_args_and_attributes(::benchmark::State& state) {
    root_logger_t root({});
    logger_facade<root_logger_t> logger(root);

    while (state.KeepRunning()) {
        logger.log(0,
            {
                {"key#1", view_t(42)},
                {"key#2", view_t(3.1415)},
                {"key#3", view_t("value")}
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
    root_logger_t root({});
    wrapper_t wrapper{root, {
        {"key#0", value_t(500)},
        {"key#1", value_t("value#1")}
    }};

    logger_facade<wrapper_t> logger(wrapper);

    while (state.KeepRunning()) {
        logger.log(0,
            {
                {"key#1", view_t(42)},
                {"key#2", view_t(3.1415)},
                {"key#3", view_t("value")}
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
    root_logger_t root({});

    wrapper_t wrapper1{root, {
        {"key#0", value_t(500)},
        {"key#1", value_t("value#1")}
    }};

    wrapper_t wrapper2{wrapper1, {
        {"key#2", value_t(500)},
        {"key#3", value_t("value#3")}
    }};

    logger_facade<wrapper_t> logger(wrapper2);

    while (state.KeepRunning()) {
        logger.log(0,
            {
                {"key#1", view_t(42)},
                {"key#2", view_t(3.1415)},
                {"key#3", view_t("value")}
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
    root_logger_t root({});

    wrapper_t wrapper1{root, {
        {"key#0", value_t(500)},
        {"key#1", value_t("value#1")}
    }};

    wrapper_t wrapper2{wrapper1, {
        {"key#2", value_t(500)},
        {"key#3", value_t("value#1")}
    }};

    wrapper_t wrapper3{wrapper2, {
        {"key#4", value_t(500)},
        {"key#5", value_t("value#5")}
    }};

    logger_facade<wrapper_t> logger(wrapper3);

    while (state.KeepRunning()) {
        logger.log(0,
            {
                {"key#6", view_t(42)},
                {"key#7", view_t(3.1415)},
                {"key#8", view_t("value")}
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

#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304
BENCHMARK(literal_with_args_using_cpp14_formatter);
#endif

BENCHMARK(literal_with_attributes);
BENCHMARK(literal_with_args_and_attributes);

BENCHMARK(literal_with_args_and_attributes_and_wrapper);
BENCHMARK(literal_with_args_and_attributes_and_two_wrappers);
BENCHMARK(literal_with_args_and_attributes_and_three_wrappers);

class threaded_fixture_t: public ::benchmark::Fixture {
protected:
    root_logger_t root;
    logger_facade<root_logger_t> logger;

public:
    threaded_fixture_t(): root({}), logger(root) {}
};

BENCHMARK_DEFINE_F(threaded_fixture_t, facade)(::benchmark::State& state) {
   while (state.KeepRunning()) {
       logger.log(0,
           {
               {"key#6", view_t(42)},
               {"key#7", view_t(3.1415)},
               {"key#8", view_t("value")}
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

BENCHMARK_REGISTER_F(threaded_fixture_t, facade)
    ->ThreadRange(1, 2 * std::thread::hardware_concurrency());

}  // namespace benchmark
}  // namespace blackhole
