#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "blackhole/logger.hpp"

namespace blackhole {

class record_t;
class scoped_t;

}  // namespace blackhole

namespace blackhole {

class root_logger_t : public logger_t {
    typedef std::function<auto(const record_t&) -> bool> filter_t;

private:
    struct sync_t;
    std::unique_ptr<sync_t> sync;

    struct inner_t;
    std::shared_ptr<inner_t> inner;

public:
    /// \note you can create a logger with no handlers, it'll just drop all messages.
    root_logger_t(std::vector<std::unique_ptr<handler_t>> handlers);
    root_logger_t(filter_t filter, std::vector<std::unique_ptr<handler_t>> handlers);

    root_logger_t(const root_logger_t& other) = delete;
    root_logger_t(root_logger_t&& other);

    ~root_logger_t();

    auto operator=(const root_logger_t& other) -> root_logger_t& = delete;
    auto operator=(root_logger_t&& other) noexcept -> root_logger_t&;

    /// \warning the filter function must be thread-safe.
    auto filter(filter_t fn) -> void;

    auto log(int severity, string_view format) const -> void;
    auto log(int severity, string_view format, attribute_pack& range) const -> void;
    auto log(int severity, string_view format, attribute_pack& range, const format_t& fn) const -> void;

    auto scoped(attributes_t attributes) const -> scoped_t;
};

}  // namespace blackhole
