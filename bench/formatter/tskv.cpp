#include <benchmark/benchmark.h>

#include <blackhole/attribute.hpp>
#include <blackhole/attributes.hpp>
#include <blackhole/stdext/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter.hpp>
#include <blackhole/formatter/tskv.hpp>
#include <blackhole/record.hpp>

#include "mod.hpp"

namespace blackhole {
namespace benchmark {

using formatter::tskv_t;

static void format_message(::benchmark::State& state) {
    auto formatter = builder<tskv_t>()
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter->format(record, writer);
        writer.inner.clear();
    }

    state.SetItemsProcessed(state.iterations());
}

static void format_leftover(::benchmark::State& state) {
    auto formatter = builder<tskv_t>()
        .build();

    const string_view message("-");
    const attribute_list attributes{{"key#1", {42}}, {"key#2", {"value#2"}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter->format(record, writer);
        writer.inner.clear();
    }

    state.SetItemsProcessed(state.iterations());
}

NBENCHMARK("formatter.tskv", format_message);
NBENCHMARK("formatter.tskv[...]", format_leftover);

}  // namespace benchmark
}  // namespace blackhole
