#include "global.hpp"

#include <blackhole/blackhole.hpp>

using namespace blackhole;

namespace blackhole {

template<class Logger>
class logger_view {};

enum class thread_safety {
    unsafe,
    reenterable,
    safe
};

template<class Logger>
class synchronized {
    Logger logger;
    mutable std::mutex mutex;
public:
    static const thread_safety thread_safety_guarantee = thread_safety::safe;

    synchronized(Logger&& logger) :
        logger(std::move(logger))
    {}

    bool enabled() const {
        std::lock_guard<std::mutex> lock(mutex);
        return logger.enabled();
    }

    void enable() {
        std::lock_guard<std::mutex> lock(mutex);
        logger.enable();
    }

    void disable() {
        std::lock_guard<std::mutex> lock(mutex);
        logger.disable();
    }

    void set_filter(filter_t&& filter) {
        std::lock_guard<std::mutex> lock(mutex);
        logger.set_filter(std::move(filter));
    }

    void add_attribute(const log::attribute_pair_t& attr) {
        std::lock_guard<std::mutex> lock(mutex);
        logger.add_attribute(attr);
    }

    void add_frontend(std::unique_ptr<base_frontend_t> frontend) {
        std::lock_guard<std::mutex> lock(mutex);
        logger.add_frontend(std::move(frontend));
    }

    void set_exception_handler(log::exception_handler_t&& handler) {
        std::lock_guard<std::mutex> lock(mutex);
        logger.set_exception_handler(std::move(handler));
    }

    template<typename... Args>
    log::record_t open_record(Args&&... args) const {
        std::lock_guard<std::mutex> lock(mutex);
        return logger.open_record(std::forward<Args>(args)...);
    }

    void push(log::record_t&& record) const {
        std::lock_guard<std::mutex> lock(mutex);
        logger.push(std::move(record));
    }
};

}

TEST(SynchronizedLogger, Class) {
    verbose_logger_t<level> log;
    synchronized<verbose_logger_t<level>> logger(std::move(log));
    UNUSED(logger);
}

TEST(Logger, Synchronized) {
    auto formatter = std::make_unique<formatter::string_t>("[%(timestamp)s] [%(severity)s]: %(message)s");
    auto sink = std::make_unique<sink::stream_t>(sink::stream_t::output_t::stdout);
    auto frontend = std::make_unique<frontend_t<formatter::string_t, sink::stream_t>>(std::move(formatter), std::move(sink));

    verbose_logger_t<level> log;
    synchronized<verbose_logger_t<level>> logger(std::move(log));
    logger.add_frontend(std::move(frontend));
    BH_LOG(logger, level::debug, "just first message");

    std::vector<std::thread> threads;
    for (int tid = 0; tid < 10; ++tid) {
        std::thread thread([&logger, tid](){
            for (int i = 0; i < 10; ++i) {
                BH_LOG(logger, level::debug, "just another message")("tid", tid);
            }
        });
        threads.push_back(std::move(thread));
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
