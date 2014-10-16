#include <ticktack/benchmark.hpp>

#include <blackhole/blackhole.hpp>

#include <blackhole/frontend/files.hpp>
#include <blackhole/sink/files.hpp>
#include <blackhole/sink/stream.hpp>
#include <blackhole/sink/null.hpp>

using namespace blackhole;

namespace { enum level_t { info }; }

namespace {

static const char FORMAT_DEFAULT[] = "[%(timestamp)s] [%(severity)s]: %(message)s %(...:[:])s";
static const char MESSAGE_LONG[] = "Something bad is going on but I can handle it";

template<class L, class F, class S>
struct initializer_t {
    std::unique_ptr<F> f;

    initializer_t(std::unique_ptr<F> f) :
        f(std::move(f))
    {}

    initializer_t(initializer_t&& other) :
        f(std::move(other.f))
    {}

    template<class... Args>
    L operator()(Args&&... args) {
        auto sink = blackhole::aux::util::make_unique<
            S
        >(std::forward<Args>(args)...);

        auto frontend = blackhole::aux::util::make_unique<
            blackhole::frontend_t<F, S>
        >(std::move(f), std::move(sink));

        L log;
        log.add_frontend(std::move(frontend));
        return log;
    }
};

template<class L, class F, class S, class... Args>
initializer_t<L, F, S>
initialize(Args&&... args) {
    auto formatter = blackhole::aux::util::make_unique<
        F
    >(std::forward<Args>(args)...);
    return initializer_t<L, F, S>(std::move(formatter));
}

} // namespace

namespace {

void run(blackhole::verbose_logger_t<level_t>& log, std::uint64_t iters, uint tid) {
    for (uint j = 0; j < iters; ++j) {
        BH_LOG(log, level_t::info, MESSAGE_LONG)("thread", tid, "id", j);
    }
}

} // namespace

BENCHMARK_RETURN(Threaded, Null) {
    static auto log = initialize<
        blackhole::verbose_logger_t<level_t>,
        blackhole::formatter::string_t,
        blackhole::sink::null_t
    >(FORMAT_DEFAULT)();
    auto it = 100000;
    auto hc = std::thread::hardware_concurrency();
    const ticktack::iteration_type iters(it);
    std::vector<std::thread> threads;
    for (uint tid = 0; tid < hc; ++tid) {
        threads.push_back(std::thread(std::bind(&run, std::ref(log), iters.v, tid)));
    }

    for (uint i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }

    return ticktack::iteration_type(it * hc);
}
