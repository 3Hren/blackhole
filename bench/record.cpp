#include <benchmark/benchmark.h>

#include <blackhole/record.hpp>

namespace blackhole {
namespace benchmark {

static
void
record(::benchmark::State& state) {
    attribute_pack pack;

    while (state.KeepRunning()) {
        record_t record(42, "GET /porn.png HTTP/1.1", pack);
        ::benchmark::DoNotOptimize(record);
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(record);

}  // namespace benchmark
}  // namespace blackhole
