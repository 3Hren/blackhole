#include <benchmark/benchmark.h>

#include <blackhole/attribute.hpp>
#include <blackhole/record.hpp>
#include <src/recordbuf.hpp>

#include "mod.hpp"

namespace blackhole {
namespace benchmark {

static void record(::benchmark::State& state) {
    const string_view message("GET /porn.png HTTP/1.1");
    const attribute_list attributes{{"key#1", "value#1"}};
    const attribute_pack pack{attributes};

    record_t record(42, message, pack);

    while (state.KeepRunning()) {
        detail::recordbuf_t owned(record);
    }

    state.SetItemsProcessed(state.iterations());
}

NBENCHMARK("record[into_owned]", record);

}  // namespace benchmark
}  // namespace blackhole
