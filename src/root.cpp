#include "blackhole/root.hpp"

#include <atomic>
#include <mutex>

#include <boost/thread/tss.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/extensions/facade.hpp"
#include "blackhole/handler.hpp"
#include "blackhole/record.hpp"
#include "blackhole/scope/manager.hpp"
#include "blackhole/scope/watcher.hpp"

#include "blackhole/detail/spinlock.hpp"

namespace blackhole {
inline namespace v1 {

using detail::spinlock_t;

namespace {

using scope::watcher_t;

class thread_manager_t : public scope::manager_t {
    boost::thread_specific_ptr<watcher_t> inner;

public:
    thread_manager_t() : inner([](watcher_t*) {}) {}

    auto get() const -> watcher_t* {
        return inner.get();
    }

    auto reset(watcher_t* value) -> void {
        inner.reset(value);
    }
};

}  // namespace

struct root_logger_t::sync_t {
    // NOTE: Actually we need to check for `std::atomic_*_explicit` overloads for `std::shared_ptr`,
    // but it seems there is no simple way to do this.
    // The given solution works both on OS X with clang compiler and on linux with GCC and clang.
    // Feel free to hack and send a PR with better solution.
#ifndef __APPLE__
    typedef spinlock_t mutex_type;
    mutable mutex_type mutex;
#endif

    thread_manager_t manager;

    auto load(const std::shared_ptr<inner_t>& source) const noexcept -> std::shared_ptr<inner_t> {
#ifdef __APPLE__
        return std::atomic_load_explicit(&source, std::memory_order_acquire);
#else
        std::lock_guard<mutex_type> lock(mutex);
        return source;
#endif
    }

    auto store(std::shared_ptr<inner_t>& source, std::shared_ptr<inner_t> value) noexcept -> void {
#ifdef __APPLE__
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

root_logger_t::root_logger_t(root_logger_t&& other) noexcept :
    sync(new sync_t),
    inner(other.sync->load(other.inner))
{
    sync->manager.reset(other.sync->manager.get());

    if (sync->manager.get()) {
        sync->manager.get()->rebind(sync->manager);
    }
}

root_logger_t::~root_logger_t() {}

auto
root_logger_t::operator=(root_logger_t&& other) noexcept -> root_logger_t& {
    if (this == &other) {
        return *this;
    }

    const auto inner = other.sync->load(other.inner);
    sync->store(this->inner, std::move(inner));

    sync->manager.reset(other.sync->manager.get());

    if (sync->manager.get()) {
        sync->manager.get()->rebind(sync->manager);
    }

    return *this;
}

auto
root_logger_t::filter(filter_t fn) -> void {
    auto inner = sync->load(this->inner);

    inner->apply([&](inner_t& inner) {
        inner.filter = std::move(fn);
    });

    sync->store(this->inner, std::move(inner));
}

namespace {

struct null_message_t {
    struct {
        constexpr auto operator()() const noexcept -> string_view {
            return {};
        }
    } supplier;
};

}  // namespace

auto root_logger_t::log(severity_t severity, const message_t& message) -> void {
    attribute_pack pack;
    log(severity, message, pack);
}

auto root_logger_t::log(severity_t severity, const message_t& message, attribute_pack& pack) -> void {
    consume(severity, message, pack, null_message_t());
}

auto root_logger_t::log(severity_t severity, const lazy_message_t& message, attribute_pack& pack) -> void {
    consume(severity, message.pattern, pack, message);
}

template<typename F>
auto root_logger_t::consume(severity_t severity, const string_view& pattern, attribute_pack& pack, const F& supplier) -> void {
    const auto inner = sync->load(this->inner);
    const auto filter = inner->apply([&](inner_t& inner) -> filter_t {
        return inner.filter;
    });

    if (sync->manager.get()) {
        sync->manager.get()->collect(pack);
    }

    record_t record(severity, pattern, pack);
    if (filter(record)) {
        const auto formatted = supplier.supplier();

        record.activate(formatted);
        for (auto& handler : inner->handlers) {
            try {
                handler->handle(record);
            } catch (const std::exception& err) {
                std::cout << "logging core error occurred: " << err.what() << std::endl;
            } catch (...) {
                std::cout << "logging core error occurred: unknown" << std::endl;
            }
        }
    }
}

auto root_logger_t::manager() -> scope::manager_t& {
    return sync->manager;
}

}  // namespace v1
}  // namespace blackhole
