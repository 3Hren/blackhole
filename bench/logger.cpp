/// Format string and write it to null.
#include <thread>

#include <benchmark/benchmark.h>

#include <blackhole/attribute.hpp>
#include <blackhole/extensions/facade.hpp>
#include <blackhole/handler.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/root.hpp>
#include <blackhole/scope/holder.hpp>
#include <blackhole/wrapper.hpp>

#include "mod.hpp"

namespace blackhole {
namespace benchmark {

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
            "[::]", "esafronov", "10/Oct/2000:13:55:36 -0700", "/porn.png", 200, 2326);
    }

    state.SetItemsProcessed(state.iterations());
}

#if (__GNUC__ >= 6 || defined(__clang__)) && defined(__cpp_constexpr) && __cpp_constexpr >= 201304
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
            "[::]", "esafronov", "10/Oct/2000:13:55:36 -0700", "/porn.png", 200, 2326);
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
        logger.log(0, "[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326", {
            {"key#1", {42}},
            {"key#2", {3.1415}},
            {"key#3", {"value"}}
        });
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_scoped_attributes(::benchmark::State& state) {
    root_logger_t root({});
    logger_facade<root_logger_t> logger(root);

    const scope::holder_t scoped(root, {
        {"key#1", {42}},
        {"key#2", {3.1415}},
        {"key#3", "value"}
    });

    while (state.KeepRunning()) {
        logger.log(0, "[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326");
    }

    state.SetItemsProcessed(state.iterations());
}

static void literal_with_scoped_attributes_everytime(::benchmark::State& state) {
    root_logger_t root({});
    logger_facade<root_logger_t> logger(root);

    while (state.KeepRunning()) {
        const scope::holder_t scoped(root, {
            {"key#1", {42}},
            {"key#2", {3.1415}},
            {"key#3", "value"}
        });

        logger.log(0, "[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326");
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_args_and_attributes(::benchmark::State& state) {
    root_logger_t root({});
    logger_facade<root_logger_t> logger(root);

    while (state.KeepRunning()) {
        logger.log(0, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
            "[::]", "esafronov", "10/Oct/2000:13:55:36 -0700", "/porn.png", 200, 2326,
            attribute_list{
                {"key#1", {42}},
                {"key#2", {3.1415}},
                {"key#3", {"value"}}
            }
        );
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_args_and_attributes_and_wrapper(::benchmark::State& state) {
    root_logger_t root({});
    wrapper_t wrapper{root, {
        {"key#0", {500}},
        {"key#1", {"value#1"}}
    }};

    logger_facade<wrapper_t> logger(wrapper);

    while (state.KeepRunning()) {
        logger.log(0, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
            "[::]", "esafronov", "10/Oct/2000:13:55:36 -0700", "/porn.png", 200, 2326,
            attribute_list{
                {"key#1", {42}},
                {"key#2", {3.1415}},
                {"key#3", {"value"}}
            }
        );
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_args_and_attributes_and_two_wrappers(::benchmark::State& state) {
    root_logger_t root({});

    wrapper_t wrapper1{root, {
        {"key#0", {500}},
        {"key#1", {"value#1"}}
    }};

    wrapper_t wrapper2{wrapper1, {
        {"key#2", {500}},
        {"key#3", {"value#3"}}
    }};

    logger_facade<wrapper_t> logger(wrapper2);

    while (state.KeepRunning()) {
        logger.log(0, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
            "[::]", "esafronov", "10/Oct/2000:13:55:36 -0700", "/porn.png", 200, 2326,
            attribute_list{
                {"key#4", {42}},
                {"key#5", {3.1415}},
                {"key#6", {"value"}}
            }
        );
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
literal_with_args_and_attributes_and_three_wrappers(::benchmark::State& state) {
    root_logger_t root({});

    wrapper_t wrapper1{root, {
        {"key#0", {500}},
        {"key#1", {"value#1"}}
    }};

    wrapper_t wrapper2{wrapper1, {
        {"key#2", {500}},
        {"key#3", {"value#3"}}
    }};

    wrapper_t wrapper3{wrapper2, {
        {"key#4", {500}},
        {"key#5", {"value#5"}}
    }};

    logger_facade<wrapper_t> logger(wrapper3);

    while (state.KeepRunning()) {
        logger.log(0, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
            "[::]", "esafronov", "10/Oct/2000:13:55:36 -0700", "/porn.png", 200, 2326,
            attribute_list{
                {"key#6", {42}},
                {"key#7", {3.1415}},
                {"key#8", {"value"}}
            }
        );
    }

    state.SetItemsProcessed(state.iterations());
}

NBENCHMARK("log.string", string);

NBENCHMARK("log.lit", literal);
NBENCHMARK("log.lit[reject]", literal_reject);
NBENCHMARK("log.lit[args: 1]", literal_with_arg);
NBENCHMARK("log.lit[args: 6]", literal_with_args);


#if (__GNUC__ >= 6 || defined(__clang__)) && defined(__cpp_constexpr) && __cpp_constexpr >= 201304
NBENCHMARK("log.lit[args: 6 + c++14::fmt]", literal_with_args_using_cpp14_formatter);
#endif

NBENCHMARK("log.lit[args: 0 + attr: 3]", literal_with_attributes);
NBENCHMARK("log.lit[args: 0 + attr(scope): 3]", literal_with_scoped_attributes);
NBENCHMARK("log.lit[args: 0 + attr(scope+): 3]", literal_with_scoped_attributes_everytime);
NBENCHMARK("log.lit[args: 6 + attr: 3]", literal_with_args_and_attributes);

NBENCHMARK("log.lit[args: 6 + attr: 3 + wrap: 1 * 2]", literal_with_args_and_attributes_and_wrapper);
NBENCHMARK("log.lit[args: 6 + attr: 3 + wrap: 2 * 2]", literal_with_args_and_attributes_and_two_wrappers);
NBENCHMARK("log.lit[args: 6 + attr: 3 + wrap: 3 * 2]", literal_with_args_and_attributes_and_three_wrappers);

class threaded_t: public ::benchmark::Fixture {
protected:
    root_logger_t root;
    logger_facade<root_logger_t> logger;

public:
    threaded_t(): root({}), logger(root) {}
};

BENCHMARK_DEFINE_F(threaded_t, facade)(::benchmark::State& state) {
    while (state.KeepRunning()) {
       logger.log(0, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
           "[::]", "esafronov", "10/Oct/2000:13:55:36 -0700", "/porn.png", 200, 2326,
           attribute_list{
               {"key#6", {42}},
               {"key#7", {3.1415}},
               {"key#8", {"value"}}
           }
       );
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_REGISTER_F(threaded_t, facade)
    ->ThreadRange(1, 2 * std::thread::hardware_concurrency());

}  // namespace benchmark
}  // namespace blackhole
