#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304

#include <benchmark/benchmark.h>

#define FMT_HEADER_ONLY
#include <cppformat/format.h>

#include <blackhole/extensions/format.hpp>

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

BENCHMARK(cpp14formatter);

}  // namespace benchmark
}  // namespace blackhole

#endif
