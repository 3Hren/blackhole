#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>

#include <blackhole/handler.hpp>
#include <blackhole/root.hpp>

using namespace blackhole;

int main(int, char** argv) {
    const auto iterations = boost::lexical_cast<int>(argv[1]);

    root_logger_t logger({});

    std::vector<boost::thread> threads;

    for (int tid = 0; tid < 4; ++tid) {
        threads.emplace_back([&] {
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
    }

    for (auto& thread : threads) {
        thread.join();
    }
}
