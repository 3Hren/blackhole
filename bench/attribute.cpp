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

static void from_owned(::benchmark::State& state) {
    blackhole::attribute::value_t owned(42);

    while (state.KeepRunning()) {
        blackhole::attribute::view_t view(owned);
        ::benchmark::DoNotOptimize(view);
    }

    state.SetItemsProcessed(state.iterations());
}

static void from_owned_string(::benchmark::State& state) {
    blackhole::attribute::value_t owned("le message");

    while (state.KeepRunning()) {
        blackhole::attribute::view_t view(owned);
        ::benchmark::DoNotOptimize(view);
    }

    state.SetItemsProcessed(state.iterations());
}

NBENCHMARK("attribute.view_t[ctor + get<i64>]", view_ctor_get_int64);
NBENCHMARK("attribute.view_t[ctor + from owned]", from_owned);
NBENCHMARK("attribute.view_t[ctor + from owned string]", from_owned_string);

}  // namespace benchmark
}  // namespace blackhole
