#include <benchmark/benchmark.h>

#include <blackhole/attribute.hpp>
#include <blackhole/attributes.hpp>
#include <blackhole/cpp17/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter/json.hpp>
#include <blackhole/record.hpp>

#include "mod.hpp"

namespace blackhole {
namespace benchmark {

using ::blackhole::formatter::json_t;

static void format_json(::benchmark::State& state) {
    json_t formatter;

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter.format(record, writer);
        writer.inner.clear();
    }

    state.SetItemsProcessed(state.iterations());
}

static void format_json_message_routed(::benchmark::State& state) {
    auto formatter = json_t::builder_t()
        .route("/fields", {"message"})
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter.format(record, writer);
        writer.inner.clear();
    }

    state.SetItemsProcessed(state.iterations());
}

static void format_json_message_routed_attr(::benchmark::State& state) {
    auto formatter = json_t::builder_t()
        .route("/fields", {"endpoint"})
        .build();

    const string_view message("value");
    const attribute_list attributes{{"endpoint", "127.0.0.1:8080"}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter.format(record, writer);
        writer.inner.clear();
    }

    state.SetItemsProcessed(state.iterations());
}

NBENCHMARK("formatter.json", format_json);
NBENCHMARK("formatter.json[route]", format_json_message_routed);
NBENCHMARK("formatter.json[route + attr: 1]", format_json_message_routed_attr);

}  // namespace benchmark
}  // namespace blackhole
