#include <benchmark/benchmark.h>

#include <blackhole/attributes.hpp>
#include <blackhole/cpp17/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/record.hpp>

namespace blackhole {
namespace benchmark {

static
void
format_literal(::benchmark::State& state) {
    formatter::string_t formatter("message: value");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter.format(record, writer);
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
format_pid(::benchmark::State& state) {
    formatter::string_t formatter("{process}");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter.format(record, writer);
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
format_procname(::benchmark::State& state) {
    formatter::string_t formatter("{process:s}");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter.format(record, writer);
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
format_message(::benchmark::State& state) {
    formatter::string_t formatter("message: {message}");

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter.format(record, writer);
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
format_timestamp(::benchmark::State& state) {
    formatter::string_t formatter("{timestamp}");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();
    writer_t writer;

    while (state.KeepRunning()) {
        formatter.format(record, writer);
    }

    state.SetItemsProcessed(state.iterations());
}

static
void
format_severity_message(::benchmark::State& state) {
    formatter::string_t formatter("{severity:d}: {message}");

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter.format(record, writer);
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(format_literal);
BENCHMARK(format_pid);
BENCHMARK(format_procname);
BENCHMARK(format_message);
BENCHMARK(format_timestamp);
BENCHMARK(format_severity_message);

}  // namespace benchmark
}  // namespace blackhole
