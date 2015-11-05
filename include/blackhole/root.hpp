#pragma once

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

template<typename T>
class transactional {
    std::shared_ptr<T> inner;
#ifndef __clang__
    mutable std::mutex mutex;
#endif

public:
    template<typename... Args>
    transactional(Args&&... args):
        inner(std::make_shared<T>(std::forward<Args>(args)...))
    {}

    auto begin() const -> std::shared_ptr<T> {
#ifdef __clang__
        return std::atomic_load_explicit(&this->inner, std::memory_order_relaxed);
#else
        std::lock_guard<std::mutex> lock(mutex);
        return inner;
#endif
    }

    auto commit(std::shared_ptr<T> value) -> void {
#ifdef __clang__
        std::atomic_store(&inner, std::move(value));
#else
        std::lock_guard<std::mutex> lock(mutex);
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
    auto log(int severity, string_view format, range_t& range) const -> void;
    auto log(int severity, string_view format, range_t& range, const format_t& fn) const -> void;

    auto scoped(attributes_t attributes) const -> scoped_t;
};

}  // namespace blackhole
