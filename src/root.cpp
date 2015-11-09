#include "blackhole/root.hpp"

#include <atomic>

#include "blackhole/extensions/facade.hpp"
#include "blackhole/handler.hpp"
#include "blackhole/record.hpp"

namespace blackhole {

struct root_logger_t::inner_t {
    filter_t filter;
    const std::vector<std::unique_ptr<handler_t>> handlers;

    inner_t(std::vector<std::unique_ptr<handler_t>> handlers):
        filter([](const record_t&) -> bool { return true; }),
        handlers(std::move(handlers))
    {}

    inner_t(filter_t filter, std::vector<std::unique_ptr<handler_t>> handlers):
        filter(std::move(filter)),
        handlers(std::move(handlers))
    {}
};

root_logger_t::root_logger_t(std::vector<std::unique_ptr<handler_t>> handlers):
    inner(std::move(handlers))
{}

root_logger_t::root_logger_t(filter_t filter, std::vector<std::unique_ptr<handler_t>> handlers):
    inner(std::move(filter), std::move(handlers))
{}

root_logger_t::~root_logger_t() {}

auto
root_logger_t::filter(filter_t fn) -> void {
    // If TSM allowed - use it...
    // __transaction_atomic {
    //     inner->filter = std::move(fn);
    // }
    // ... otherwise emulate
    // inner->write_transaction([&](inner_t& inner) {
    //     inner.filter->write_transaction([&](filter_t& filter) {
    //         filter = std::move(fn);
    //     });
    // });

    // 1. Copy the inner ptr (using atomic load + copy ctor).
    // auto inner = std::atomic_load(&this->inner);
    auto inner = this->inner.begin();
    // TODO: Here the RC! Use RCU for function to avoid this.
    // 2. Set new filter.
    inner->filter = std::move(fn);

    // 3. Atomically reset inner ptr.
    // std::atomic_store(&this->inner, std::move(inner));
    this->inner.commit(std::move(inner));
    // 4. Test with valgrind (write into log from N threads, reset filter from M threads)!
}

auto
root_logger_t::log(int severity, string_view message) const -> void {
    range_t range;
    log(severity, message, range);
}

auto
root_logger_t::log(int severity, string_view message, range_t& range) const -> void {
    (void)severity;
    (void)message;
    (void)range;
    // const auto inner = std::atomic_load(&this->inner);
    const auto inner = this->inner.begin();

    record_t record;
    if (inner->filter(record)) {
        for (auto& handler : inner->handlers) {
            handler->execute(record);
        }
    }
}

auto
root_logger_t::log(int severity, string_view format, range_t& range, const format_t& fn) const -> void {
    (void)severity;
    (void)format;
    (void)range;
    // const auto inner = std::atomic_load(&this->inner);
    const auto inner = this->inner.begin();

    record_t record;
    if (inner->filter(record)) {
        writer_t wr;
        fn(wr);

        for (auto& handler : inner->handlers) {
            handler->execute(record);
        }
    }
}

}  // namespace blackhole
