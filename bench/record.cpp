#include <benchmark/benchmark.h>

#include <blackhole/attribute.hpp>
#include <blackhole/record.hpp>

#include "mod.hpp"

namespace blackhole {
namespace benchmark {

static void record(::benchmark::State& state) {
    const string_view message("GET /porn.png HTTP/1.1");
    const attribute_pack pack;

    while (state.KeepRunning()) {
        record_t(42, message, pack);
    }

    state.SetItemsProcessed(state.iterations());
}

NBENCHMARK("record[ctor]", record);

}  // namespace benchmark
}  // namespace blackhole
