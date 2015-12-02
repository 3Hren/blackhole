#include <benchmark/benchmark.h>

#include <blackhole/attribute.hpp>

namespace blackhole {
namespace benchmark {
namespace {

void view_ctor_int64(::benchmark::State& state) {
    while (state.KeepRunning()) {
        blackhole::attribute::view_t v(42);
        blackhole::attribute::get<std::int64_t>(v);
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(view_ctor_int64);

}  // namespace
}  // namespace benchmark
}  // namespace blackhole
