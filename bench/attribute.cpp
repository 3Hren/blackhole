#include <benchmark/benchmark.h>

#include <blackhole/attribute.hpp>

#include "mod.hpp"

namespace blackhole {
namespace benchmark {

static void view_ctor_get_int64(::benchmark::State& state) {
    while (state.KeepRunning()) {
        blackhole::attribute::view_t v(42);
        blackhole::attribute::get<std::int64_t>(v);
    }

    state.SetItemsProcessed(state.iterations());
}

NBENCHMARK("attribute.view_t[ctor + get<i64>]", view_ctor_get_int64);

}  // namespace benchmark
}  // namespace blackhole
