#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "blackhole/logger.hpp"

namespace blackhole {
inline namespace v1 {

class handler_t;
class record_t;
class scoped_t;

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

    auto log(int severity, const string_view& message) -> void;
    auto log(int severity, const string_view& message, attribute_pack& pack) -> void;
    auto log(int severity, const lazy_message_t& message, attribute_pack& pack) -> void;

    auto context() -> boost::thread_specific_ptr<scoped_t>*;

private:
    template<typename F>
    auto consume(int severity, const string_view& pattern, attribute_pack& pack, const F& fn) -> void;
};

}  // namespace v1
}  // namespace blackhole
