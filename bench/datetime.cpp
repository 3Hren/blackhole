#include <benchmark/benchmark.h>

#include <cppformat/format.h>

#include <blackhole/detail/datetime.hpp>

namespace blackhole {
namespace benchmark {

static void datetime_strftime(::benchmark::State& state) {
    std::time_t time = std::time(0);
    std::tm tm;
    localtime_r(&time, &tm);

    while (state.KeepRunning()) {
        char buffer[64];
        strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
    }

    state.SetItemsProcessed(state.iterations());
}

static void datetime_strftime_with_locale(::benchmark::State& state) {
    std::time_t time = std::time(0);
    std::tm tm;
    localtime_r(&time, &tm);

    while (state.KeepRunning()) {
        char buffer[64];
        strftime(buffer, 64, "%c", &tm);
    }

    state.SetItemsProcessed(state.iterations());
}

static void datetime_wheel(::benchmark::State& state) {
    blackhole::detail::datetime::generator_t generator(
        blackhole::detail::datetime::make_generator("%Y-%m-%d %H:%M:%S")
    );

    fmt::MemoryWriter wr;

    std::time_t time = std::time(0);
    std::tm tm;
    localtime_r(&time, &tm);

    while (state.KeepRunning()) {
        generator(wr, tm);
        wr.clear();
    }

    state.SetItemsProcessed(state.iterations());
}

static void datetime_wheel_with_locale(::benchmark::State& state) {
    blackhole::detail::datetime::generator_t generator(
        blackhole::detail::datetime::make_generator("%c")
    );

    fmt::MemoryWriter wr;

    std::time_t time = std::time(0);
    std::tm tm;
    localtime_r(&time, &tm);

    while (state.KeepRunning()) {
        generator(wr, tm);
        wr.clear();
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(datetime_strftime);
BENCHMARK(datetime_strftime_with_locale);
BENCHMARK(datetime_wheel);
BENCHMARK(datetime_wheel_with_locale);

}  // namespace benchmark
}  // namespace blackhole
