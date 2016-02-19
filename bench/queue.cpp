#include <memory>
#include <thread>

#include <boost/asio/io_service.hpp>

#include <benchmark/benchmark.h>

#include "mod.hpp"

namespace blackhole {
namespace benchmark {

static void io_post(::benchmark::State& state) {
    boost::asio::io_service loop;
    std::unique_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(loop));

    std::uint64_t counter = 0;
    std::thread thread([&] {
        loop.run();
    });

    while (state.KeepRunning()) {
        loop.post([&] {
            ++counter;
        });
    }

    work.reset();
    thread.join();

    state.SetItemsProcessed(state.iterations());
}

NBENCHMARK("I/O post", io_post);

}  // namespace benchmark
}  // namespace blackhole
