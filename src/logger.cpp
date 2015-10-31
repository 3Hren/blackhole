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
};

logger_t::logger_t(std::vector<std::unique_ptr<handler_t>> handlers):
    inner(std::make_shared<inner_t>(std::move(handlers)))
{}

logger_t::~logger_t() {}

auto
logger_t::filter(filter_type fn) -> void {
    auto inner = std::atomic_load(&this->inner);
    // TODO: Here the RC! Use RCU for function to avoid this.
    inner->filter = std::move(fn);

    std::atomic_store(&this->inner, std::move(inner));
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
    const auto inner = std::atomic_load(&this->inner);

    record_t record;
    for (auto& handler : inner->handlers) {
        handler->execute(record);
    }
}

}  // namespace blackhole
