#include "blackhole/log.hpp"

#include <atomic>

namespace blackhole {

struct log_t::inner_t {
    log_t::filter_type filter;

    inner_t():
        filter([](const record_t&) -> bool { return true; })
    {}
};

log_t::log_t():
    inner(std::make_shared<inner_t>())
{}

log_t::~log_t() {}

auto
log_t::filter(filter_type fn) -> void {
    auto inner = std::atomic_load(&this->inner);
    inner->filter = std::move(fn);

    std::atomic_store(&this->inner, std::move(inner));
}

void
log_t::info(string_view message) {
    const auto inner = std::atomic_load(&this->inner);

    record_t record;
    if (inner->filter(record)) {
        // for each handle do handle->execute().
    }
}

void
log_t::info(string_view message, const attributes_t& attributes) {
    auto inner = std::atomic_load(&this->inner);

    for (const auto& attribute : attributes) {
    }
}

}  // namespace blackhole
