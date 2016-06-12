#include <benchmark/benchmark.h>

#include <blackhole/attribute.hpp>
#include <blackhole/attributes.hpp>
#include <blackhole/cpp17/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/record.hpp>

#include "mod.hpp"

namespace blackhole {
namespace benchmark {

using formatter::string_t;

static void format_literal(::benchmark::State& state) {
    auto formatter = builder<string_t>("message: value")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter->format(record, writer);
        writer.inner.clear();
    }

    state.SetItemsProcessed(state.iterations());
}

static void format_pid(::benchmark::State& state) {
    auto formatter = builder<string_t>("{process}")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter->format(record, writer);
        writer.inner.clear();
    }

    state.SetItemsProcessed(state.iterations());
}

static void format_procname(::benchmark::State& state) {
    auto formatter = builder<string_t>("{process:s}")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter->format(record, writer);
        writer.inner.clear();
    }

    state.SetItemsProcessed(state.iterations());
}

static void format_tid(::benchmark::State& state) {
    auto formatter = builder<string_t>("{thread}")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter->format(record, writer);
        writer.inner.clear();
    }

    state.SetItemsProcessed(state.iterations());
}

static void format_message(::benchmark::State& state) {
    auto formatter = builder<string_t>("message: {message}")
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

static void format_timestamp(::benchmark::State& state) {
    auto formatter = builder<string_t>("{timestamp}")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();
    writer_t writer;

    while (state.KeepRunning()) {
        formatter->format(record, writer);
        writer.inner.clear();
    }

    state.SetItemsProcessed(state.iterations());
}

static void format_leftover(::benchmark::State& state) {
    auto formatter = builder<string_t>("{...:{{name}={value}:p}s}")
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

static void format_severity_message(::benchmark::State& state) {
    auto formatter = builder<string_t>("{severity:d}: {message}")
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

NBENCHMARK("formatter.string[lit]", format_literal);
NBENCHMARK("formatter.string[pid]", format_pid);
NBENCHMARK("formatter.string[tid]", format_tid);
NBENCHMARK("formatter.string[procname]", format_procname);
NBENCHMARK("formatter.string[message]", format_message);
NBENCHMARK("formatter.string[timestamp]", format_timestamp);
NBENCHMARK("formatter.string[severity + message]", format_severity_message);
NBENCHMARK("formatter.string[...]", format_leftover);

}  // namespace benchmark
}  // namespace blackhole
