#include "blackhole/root.hpp"

#include <atomic>
#include <mutex>

#include "blackhole/extensions/facade.hpp"
#include "blackhole/handler.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/spinlock.hpp"

namespace blackhole {

using detail::spinlock_t;

struct root_logger_t::sync_t {
    typedef spinlock_t mutex_type;

    auto clone(const std::shared_ptr<inner_t>& inner) const -> std::shared_ptr<inner_t> {
#ifdef __clang__
        return std::atomic_load_explicit(&inner, std::memory_order_acquire);
#else
        std::lock_guard<mutex_type> lock(mutex);
        return inner;
#endif
    }

    auto commit(std::shared_ptr<inner_t>& inner, std::shared_ptr<inner_t> value) -> void {
#ifdef __clang__
        std::atomic_store_explicit(&inner, std::move(value), std::memory_order_release);
#else
        std::lock_guard<mutex_type> lock(mutex);
        inner = std::move(value);
#endif
    }

private:
    mutable mutex_type mutex;
};

struct root_logger_t::inner_t {
    typedef spinlock_t mutex_type;

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

    template<typename F>
    auto apply(F&& fn) -> decltype(fn(std::declval<inner_t&>())) {
        std::lock_guard<mutex_type> lock(mutex);
        return fn(*this);
    }

private:
    mutable mutex_type mutex;
};

root_logger_t::root_logger_t(std::vector<std::unique_ptr<handler_t>> handlers):
    sync(new sync_t),
    inner(std::make_shared<inner_t>(std::move(handlers)))
{}

root_logger_t::root_logger_t(filter_t filter, std::vector<std::unique_ptr<handler_t>> handlers):
    sync(new sync_t),
    inner(std::make_shared<inner_t>(std::move(filter), std::move(handlers)))
{}

root_logger_t::~root_logger_t() {}

auto
root_logger_t::filter(filter_t fn) -> void {
    auto inner = sync->clone(this->inner);

    inner->apply([&](inner_t& inner) {
        inner.filter = std::move(fn);
    });

    sync->commit(this->inner, std::move(inner));
}

auto
root_logger_t::log(int severity, string_view message) const -> void {
    attribute_pack range;
    log(severity, message, range);
}

auto
root_logger_t::log(int severity, string_view message, attribute_pack& range) const -> void {
    (void)severity;
    (void)message;
    (void)range;

    const auto inner = sync->clone(this->inner);
    const auto filter = inner->apply([&](inner_t& inner) -> filter_t {
        return inner.filter;
    });

    record_t record;
    if (filter(record)) {
        for (auto& handler : inner->handlers) {
            handler->execute(record);
        }
    }
}

auto
root_logger_t::log(int severity, string_view format, attribute_pack& range, const format_t& fn) const -> void {
    (void)severity;
    (void)format;
    (void)range;

    const auto filter = sync->clone(inner)->apply([&](inner_t& inner) -> filter_t {
        return inner.filter;
    });

    record_t record;
    if (filter(record)) {
        writer_t wr;
        fn(wr);

        for (auto& handler : inner->handlers) {
            handler->execute(record);
        }
    }
}

auto
root_logger_t::operator=(root_logger_t&& other) -> root_logger_t& {
    if (this == &other) {
        return *this;
    }

    const auto inner = std::move(other.sync->clone(other.inner));
    sync->commit(this->inner, std::move(inner));

    return *this;
}

}  // namespace blackhole
