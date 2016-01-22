#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304

#include <benchmark/benchmark.h>

#include <blackhole/extensions/format.hpp>
#include <blackhole/extensions/metaformat.hpp>
#include <blackhole/extensions/metaformat2.hpp>

#include "mod.hpp"

namespace blackhole {
namespace benchmark {

static
void
cpp14formatter(::benchmark::State& state) {
    while (state.KeepRunning()) {
        constexpr auto formatter = blackhole::detail::formatter<
            blackhole::detail::literal_count("{} - {} [{}] 'GET {} HTTP/1.0' {} {}")
        >("{} - {} [{}] 'GET {} HTTP/1.0' {} {}");

        fmt::MemoryWriter wr;
        formatter.format(wr, "[::]", "esafronov", "10/Oct/2000:13:55:36 -0700", "/porn.png", 200, 2326);
    }

    state.SetItemsProcessed(state.iterations());
}

static auto cpp14tokenizer_baseline(::benchmark::State& state) -> void {
    while (state.KeepRunning()) {
        fmt::MemoryWriter wr;
        wr << "[::]";
        wr << " - ";
        wr << "esafronov";
        wr << " [";
        wr << "10/Oct/2000:13:55:36 -0700";
        wr << "] 'GET ";
        wr << "/porn.png";
        wr << " HTTP/1.0' ";
        wr << 200;
        wr << " ";
        wr << 2326;
    }

    state.SetItemsProcessed(state.iterations());
}

static auto cpp14tokenizer(::benchmark::State& state) -> void {
    while (state.KeepRunning()) {
        constexpr auto t = experimental::detail::tokenizer<6, 11>("{} - {} [{}] 'GET {} HTTP/1.0' {} {}");

        fmt::MemoryWriter wr;
        t.format(wr, "[::]", "esafronov", "10/Oct/2000:13:55:36 -0700", "/porn.png", 200, 2326);
    }

    state.SetItemsProcessed(state.iterations());
}

NBENCHMARK("c++14::fmt", cpp14formatter);
NBENCHMARK("c++14::baseline", cpp14tokenizer_baseline);
NBENCHMARK("c++14::tokenize", cpp14tokenizer);

}  // namespace benchmark
}  // namespace blackhole

#endif
