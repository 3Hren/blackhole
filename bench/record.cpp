#include <benchmark/benchmark.h>

#include <blackhole/record.hpp>

namespace blackhole {
namespace benchmark {

static
void
record(::benchmark::State& state) {
    const string_view message("GET /porn.png HTTP/1.1");
    const attribute_pack pack;

    while (state.KeepRunning()) {
        record_t(42, message, pack);
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(record);

}  // namespace benchmark
}  // namespace blackhole
