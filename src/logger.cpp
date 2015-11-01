#include "blackhole/logger.hpp"

#include <atomic>

namespace blackhole {

struct logger_t::inner_t {
    filter_type filter;
    const std::vector<std::unique_ptr<handler_t>> handlers;

    inner_t(std::vector<std::unique_ptr<handler_t>> handlers):
        filter([](const record_t&) -> bool { return true; }),
        handlers(std::move(handlers))
    {}

    inner_t(filter_type filter, std::vector<std::unique_ptr<handler_t>> handlers):
        filter(std::move(filter)),
        handlers(std::move(handlers))
    {}
};

logger_t::logger_t(std::vector<std::unique_ptr<handler_t>> handlers):
    inner(std::make_shared<inner_t>(std::move(handlers)))
{}

logger_t::logger_t(filter_type filter, std::vector<std::unique_ptr<handler_t>> handlers):
    inner(std::make_shared<inner_t>(std::move(filter), std::move(handlers)))
{}

logger_t::~logger_t() {}

auto
logger_t::filter(filter_type fn) -> void {
    // 1. Copy the inner ptr (using atomic load + copy ctor).
    auto inner = std::atomic_load(&this->inner);
    // TODO: Here the RC! Use RCU for function to avoid this.
    // 2. Set new filter.
    inner->filter = std::move(fn);

    // 3. Atomically reset inner ptr.
    std::atomic_store(&this->inner, std::move(inner));
    // 4. Test with valgrind (write into log from N threads, reset filter from M threads)!
}

auto
logger_t::log(int severity, string_view message) const -> void {
    const auto inner = std::atomic_load(&this->inner);

    record_t record;
    if (inner->filter(record)) {
        for (auto& handler : inner->handlers) {
            handler->execute(record);
        }
    }
}

auto
logger_t::log(int severity, string_view format, const format_callback& callback) const -> void {
    const auto inner = std::atomic_load(&this->inner);

    record_t record;
    if (inner->filter(record)) {
        cppformat::MemoryWriter wr;
        callback(wr);

        for (auto& handler : inner->handlers) {
            handler->execute(record);
        }
    }
}

auto
logger_t::log(int severity, const range_type& range, string_view format, const format_callback& callback) const -> void {
    const auto inner = std::atomic_load(&this->inner);

    record_t record;
    if (inner->filter(record)) {
        cppformat::MemoryWriter wr;
        callback(wr);

        for (auto& handler : inner->handlers) {
            handler->execute(record);
        }
    }
}

void
logger_t::info(string_view message) {
    const auto inner = std::atomic_load(&this->inner);

    record_t record;
    if (inner->filter(record)) {
        for (auto& handler : inner->handlers) {
            handler->execute(record);
        }
    }
}

void
logger_t::info(string_view message, const attributes_t& attributes) {
    // Copy the inner ptr to be alive even if swapped. Work only with it.
    const auto inner = std::atomic_load(&this->inner);

    record_t record;
    for (auto& handler : inner->handlers) {
        handler->execute(record);
    }
}

}  // namespace blackhole
