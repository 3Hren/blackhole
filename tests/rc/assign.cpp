#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>

#include <blackhole/attribute.hpp>
#include <blackhole/handler.hpp>
#include <blackhole/root.hpp>
#include <blackhole/scoped.hpp>

using namespace blackhole;

int main(int, char** argv) {
    const auto thread_num = boost::lexical_cast<int>(argv[1]);
    const auto iterations = boost::lexical_cast<int>(argv[2]);

    root_logger_t logger({});

    std::vector<boost::thread> threads;

    for (int tid = 0; tid < thread_num; ++tid) {
        threads.emplace_back([&] {
            const scoped_t scoped(logger, {{"key#1", {42}}});
            for (int i = 0; i < iterations; ++i) {
                logger.log(0, "GET /porn.png HTTP/1.1");
            }
        });
    }

    bool allow = false;
    for (int i = 0; i < iterations; ++i) {
        allow = !allow;
        logger = root_logger_t([=](const record_t&) -> bool {
            return allow;
        }, {});
        const scoped_t scoped(logger, {{"key#2", {100}}});
    }

    for (auto& thread : threads) {
        thread.join();
    }
}
