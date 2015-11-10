#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "blackhole/logger.hpp"

namespace blackhole {

class record_t;
class scoped_t;

}  // namespace blackhole

namespace blackhole {

class spinlock {
private:
    enum state_t {
        locked = 0,
        unlocked = 1
    };

    std::atomic<int> state;

public:
    constexpr spinlock() noexcept:
        state(unlocked)
    {}

    auto lock() noexcept -> void {
        while (state.exchange(locked, std::memory_order_acquire) == locked) {
            // Ok... try again.
        }
    }

    auto unlock() noexcept -> void {
        state.store(unlocked, std::memory_order_release);
    }
};

template<typename T>
class transactional {
    std::shared_ptr<T> inner;
#ifndef __clang__
    typedef spinlock mutex_type;
    mutable mutex_type mutex;
#endif

public:
    template<typename... Args>
    transactional(Args&&... args):
        inner(std::make_shared<T>(std::forward<Args>(args)...))
    {}

    auto begin() const -> std::shared_ptr<T> {
#ifdef __clang__
        return std::atomic_load_explicit(&this->inner, std::memory_order_acquire);
#else
        std::lock_guard<mutex_type> lock(mutex);
        return inner;
#endif
    }

    auto commit(std::shared_ptr<T> value) -> void {
#ifdef __clang__
        std::atomic_store_explicit(&inner, std::move(value), std::memory_order_release);
#else
        std::lock_guard<mutex_type> lock(mutex);
        inner = std::move(value);
#endif
    }
};

class root_logger_t : public logger_t {
    typedef std::function<auto(const record_t&) -> bool> filter_t;

private:
    struct inner_t;
    transactional<inner_t> inner;

public:
    /// \note you can create a logger with no handlers, it'll just drop all messages.
    root_logger_t(std::vector<std::unique_ptr<handler_t>> handlers);
    root_logger_t(filter_t filter, std::vector<std::unique_ptr<handler_t>> handlers);

    root_logger_t(const root_logger_t& other) = delete;
    root_logger_t(root_logger_t&& other);

    ~root_logger_t();

    auto operator=(const root_logger_t& other) -> root_logger_t& = delete;
    auto operator=(root_logger_t&& other) -> root_logger_t&;

    auto filter(filter_t fn) -> void;

    auto log(int severity, string_view format) const -> void;
    auto log(int severity, string_view format, attribute_pack& range) const -> void;
    auto log(int severity, string_view format, attribute_pack& range, const format_t& fn) const -> void;

    auto scoped(attributes_t attributes) const -> scoped_t;
};

}  // namespace blackhole
