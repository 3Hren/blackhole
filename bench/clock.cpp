#include <benchmark/benchmark.h>

#include <chrono>

#ifdef __linux__
#   include <sys/time.h>
#endif

namespace blackhole {
namespace benchmark {

#ifdef __linux__
static
void
monotonic_coarse(::benchmark::State& state) {
    while (state.KeepRunning()) {
        struct timespec ts;
        ::clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
monotonic_precise(::benchmark::State& state) {
    while (state.KeepRunning()) {
        struct timespec ts;
        ::clock_gettime(CLOCK_MONOTONIC, &ts);
    }

    state.SetItemsProcessed(state.iterations());
}
#endif

static
void
system_clock(::benchmark::State& state) {
    while (state.KeepRunning()) {
        std::chrono::system_clock::now();
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
high_resolution_clock(::benchmark::State& state) {
    while (state.KeepRunning()) {
        std::chrono::high_resolution_clock::now();
    }

    state.SetItemsProcessed(state.iterations());
}

#ifdef __linux__
BENCHMARK(monotonic_coarse);
BENCHMARK(monotonic_precise);
#endif

BENCHMARK(system_clock);
BENCHMARK(high_resolution_clock);

}  // namespace benchmark
}  // namespace blackhole
