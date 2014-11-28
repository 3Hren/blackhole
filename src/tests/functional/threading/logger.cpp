///
/// This source file contains tests, that are planned to be used with thread sanitizer or Valgrind.
///

#include <blackhole/logger.hpp>
#include <blackhole/macro.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/sink/stream.hpp>

#include "../../global.hpp"

using namespace blackhole;

namespace {

static inline void run(logger_base_t& log, int) {
    for (int i = 0; i < 10000; ++i) {
        if (auto record = log.open_record()) {
            aux::logger::make_pusher(log, record, "le message");
        }
    }
}

} // namespace

TEST(SanitizeThread, Logger) {
    using aux::util::make_unique;

    typedef formatter::string_t                   formatter_type;
    typedef sink::stream_t                        sink_type;
    typedef frontend_t<formatter_type, sink_type> frontend_type;
    typedef logger_base_t                         logger_type;

    std::ostringstream stream;

    auto formatter = make_unique<formatter_type>("%(message)s %(...:[:])s");
    auto sink      = make_unique<sink_type>(stream);
    auto frontend  = make_unique<frontend_type>(std::move(formatter), std::move(sink));
    logger_type log;
    log.add_frontend(std::move(frontend));

    std::vector<std::thread> threads;

    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(std::bind(&run, std::ref(log), i));
    }

    for (auto it = threads.begin(); it != threads.end(); ++it) {
        if (it->joinable()) {
            it->join();
        }
    }
}
