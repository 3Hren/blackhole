#include <ticktack/benchmark.hpp>

#include <blackhole/blackhole.hpp>

#include <blackhole/frontend/files.hpp>
#include <blackhole/sink/files.hpp>
#include <blackhole/sink/null.hpp>

#include "../util.hpp"

using namespace blackhole;

namespace { enum level_t { info }; }

#define MESSAGE_LONG "Something bad is going on but I can handle it"

static const std::string FORMAT_BASE    = "[%(timestamp)s]: %(message)s";
static const std::string FORMAT_VERBOSE = "[%(timestamp)s] [%(severity)s]: %(message)s";

namespace {

void run(blackhole::verbose_logger_t<level_t>& log, std::uint64_t iters, uint tid) {
    for (uint j = 0; j < iters; ++j) {
        BH_LOG(log, level_t::info, MESSAGE_LONG)("thread", tid, "id", j);
    }
}

ticktack::iteration_type null(std::size_t concurrency) {
    static auto log = initialize<
        blackhole::verbose_logger_t<level_t>,
        blackhole::formatter::string_t,
        blackhole::sink::null_t
    >(FORMAT_VERBOSE)()();

    const ticktack::iteration_type iters(100000);
    std::vector<std::thread> threads;
    for (uint tid = 0; tid < concurrency; ++tid) {
        threads.push_back(std::thread(std::bind(&run, std::ref(log), iters.v, tid)));
    }

    for (uint i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }

    return ticktack::iteration_type(iters.v * concurrency);
}

ticktack::iteration_type file(std::size_t concurrency) {
    blackhole::sink::files::config_t<> config("blackhole.log");
    static auto log = initialize<
        blackhole::verbose_logger_t<level_t>,
        blackhole::formatter::string_t,
        blackhole::sink::files_t<>
    >(FORMAT_VERBOSE)(config)();

    const ticktack::iteration_type iters(1000000 / concurrency);
    std::vector<std::thread> threads;
    for (uint tid = 0; tid < concurrency; ++tid) {
        threads.push_back(std::thread(std::bind(&run, std::ref(log), iters.v, tid)));
    }

    for (uint i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }

    return ticktack::iteration_type(iters.v * concurrency);
}

} // namespace

BENCHMARK_BOUND(Threaded, null, 1);
BENCHMARK_BOUND(Threaded, null, boost::thread::hardware_concurrency());

BENCHMARK_BOUND(Threaded, file, 1);
BENCHMARK_BOUND(Threaded, file, boost::thread::hardware_concurrency());
