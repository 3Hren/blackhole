#include <benchmark/benchmark.h>

#include <blackhole/attribute.hpp>
#include <blackhole/attributes.hpp>
#include <blackhole/cpp17/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/record.hpp>

#include "mod.hpp"

namespace blackhole {
namespace benchmark {

static void format_literal(::benchmark::State& state) {
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

static void format_pid(::benchmark::State& state) {
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

static void format_procname(::benchmark::State& state) {
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

static void format_tid(::benchmark::State& state) {
    formatter::string_t formatter("{thread}");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter.format(record, writer);
    }

    state.SetItemsProcessed(state.iterations());
}

static void format_message(::benchmark::State& state) {
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

static void format_timestamp(::benchmark::State& state) {
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

static void format_leftover(::benchmark::State& state) {
    formatter::string_t formatter("{...}", {
        {"...", formatter::option::leftover_t{false, "[", "]", "{k}: {v}", ", "}}
    });

    const string_view message("-");
    const attribute_list attributes{{"key#1", {42}}, {"key#2", {"value#2"}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter.format(record, writer);
    }

    state.SetItemsProcessed(state.iterations());
}

static void format_leftover_unique(::benchmark::State& state) {
    formatter::string_t formatter("{...}", {
        {"...", formatter::option::leftover_t{true, "[", "]", "{k}: {v}", ", "}}
    });

    const string_view message("-");
    const attribute_list attributes{{"key#1", {42}}, {"key#1", {"value#2"}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;

    while (state.KeepRunning()) {
        formatter.format(record, writer);
    }

    state.SetItemsProcessed(state.iterations());
}

static void format_severity_message(::benchmark::State& state) {
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

NBENCHMARK("formatter.string[lit]", format_literal);
NBENCHMARK("formatter.string[pid]", format_pid);
NBENCHMARK("formatter.string[tid]", format_tid);
NBENCHMARK("formatter.string[procname]", format_procname);
NBENCHMARK("formatter.string[message]", format_message);
NBENCHMARK("formatter.string[timestamp]", format_timestamp);
NBENCHMARK("formatter.string[severity + message]", format_severity_message);
NBENCHMARK("formatter.string[...]", format_leftover);
NBENCHMARK("formatter.string[... + unique]", format_leftover_unique);

}  // namespace benchmark
}  // namespace blackhole
