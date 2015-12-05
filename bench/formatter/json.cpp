#include <benchmark/benchmark.h>

#include <blackhole/attributes.hpp>
#include <blackhole/cpp17/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter/json.hpp>
#include <blackhole/record.hpp>

namespace blackhole {
namespace benchmark {

static void format_json_1(::benchmark::State& state) {
    formatter::json_t formatter(formatter::routing_t().spec("/fields", {"endpoint"}));

    const string_view message("value");
    const attribute_list attributes{{"endpoint", "127.0.0.1:8080"}};
    const attribute_pack pack{};//attributes};
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter.format(record, writer);
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(format_json_1);

}  // namespace benchmark
}  // namespace blackhole
