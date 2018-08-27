#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "blackhole/forward.hpp"
#include "blackhole/logger.hpp"

namespace blackhole {
inline namespace v1 {

class handler_t;
class record_t;

}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {

class root_logger_t : public logger_t {
public:
    typedef std::function<auto(const record_t&) -> bool> filter_t;

private:
    struct sync_t;
    std::unique_ptr<sync_t> sync;

    struct inner_t;
    std::shared_ptr<inner_t> inner;

public:
    /// Constructs a root level logger with the given handlers.
    ///
    /// \note you can create a logger with no handlers, it'll just drop all messages.
    root_logger_t(std::vector<std::unique_ptr<handler_t>> handlers);

    /// Constructs a root level logger with the given filtering function and handlers.
    ///
    /// \overload
    /// \note you can create a logger with no handlers, it'll just drop all messages.
    root_logger_t(filter_t filter, std::vector<std::unique_ptr<handler_t>> handlers);

    root_logger_t(const root_logger_t& other) = delete;

    /// Constructs a root level logger by consuming another existing logger.
    root_logger_t(root_logger_t&& other) noexcept;

    ~root_logger_t();

    auto operator=(const root_logger_t& other) -> root_logger_t& = delete;
    auto operator=(root_logger_t&& other) noexcept -> root_logger_t&;

    /// Replaces the current logger filter function with the given one.
    ///
    /// Any logging event for which the filter function returns `false` is rejected.
    ///
    /// \warning the function must be thread-safe.
    auto filter(filter_t fn) -> void;

    auto log(severity_t severity, const message_t& message) -> void;
    auto log(severity_t severity, const message_t& message, attribute_pack& pack) -> void;
    auto log(severity_t severity, const lazy_message_t& message, attribute_pack& pack) -> void;

    auto manager() -> scope::manager_t&;

private:
    template<typename F>
    auto consume(severity_t severity, const string_view& pattern, attribute_pack& pack, const F& fn) -> void;
};

template<>
class builder<root_logger_t> {
public:
    typedef root_logger_t result_type;

    class inner_t;

private:
    std::unique_ptr<inner_t, deleter_t> d;

public:
    builder();

    auto add(std::unique_ptr<handler_t> handler) & -> builder&;
    auto add(std::unique_ptr<handler_t> handler) && -> builder&&;

    auto build() && -> std::unique_ptr<result_type>;
};

}  // namespace v1
}  // namespace blackhole
