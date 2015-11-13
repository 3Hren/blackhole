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
#ifndef __clang__
    typedef spinlock_t mutex_type;
    mutable mutex_type mutex;
#endif

    auto load(const std::shared_ptr<inner_t>& source) const noexcept -> std::shared_ptr<inner_t> {
#ifdef __clang__
        return std::atomic_load_explicit(&source, std::memory_order_acquire);
#else
        std::lock_guard<mutex_type> lock(mutex);
        return source;
#endif
    }

    auto store(std::shared_ptr<inner_t>& source, std::shared_ptr<inner_t> value) noexcept -> void {
#ifdef __clang__
        std::atomic_store_explicit(&source, std::move(value), std::memory_order_release);
#else
        std::lock_guard<mutex_type> lock(mutex);
        source = std::move(value);
#endif
    }
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
    auto inner = sync->load(this->inner);

    inner->apply([&](inner_t& inner) {
        inner.filter = std::move(fn);
    });

    sync->store(this->inner, std::move(inner));
}

auto
root_logger_t::log(int severity, string_view pattern) -> void {
    attribute_pack pack;
    log(severity, pattern, pack);
}

auto
root_logger_t::log(int severity, string_view pattern, attribute_pack& pack) -> void {
    const auto inner = sync->load(this->inner);
    const auto filter = inner->apply([&](inner_t& inner) -> filter_t {
        return inner.filter;
    });

    record_t record(severity, pattern, pack);
    if (filter(record)) {
        for (auto& handler : inner->handlers) {
            handler->execute(record);
        }
    }
}

auto
root_logger_t::log(int severity, string_view pattern, attribute_pack& pack, const format_t& fn) -> void {
    const auto inner = sync->load(this->inner);
    const auto filter = inner->apply([&](inner_t& inner) -> filter_t {
        return inner.filter;
    });

    record_t record(severity, pattern, pack);
    if (filter(record)) {
        writer_t writer;
        fn(writer);
        record.activate({writer.inner.data(), writer.inner.size()});

        for (auto& handler : inner->handlers) {
            handler->execute(record);
        }
    }
}

auto
root_logger_t::operator=(root_logger_t&& other) noexcept -> root_logger_t& {
    if (this == &other) {
        return *this;
    }

    const auto inner = std::move(other.sync->load(other.inner));
    sync->store(this->inner, std::move(inner));

    return *this;
}

}  // namespace blackhole
