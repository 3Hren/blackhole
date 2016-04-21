#pragma once

#include <syslog.h>

#include <memory>
#include <vector>

#include "blackhole/forward.hpp"
#include "blackhole/sink.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

class syslog_t : public sink_t {
    class inner_t;
    std::unique_ptr<inner_t> inner;

public:
    syslog_t();
    syslog_t(const syslog_t& other) = delete;
    syslog_t(syslog_t&& other);

    ~syslog_t();

    auto option() const noexcept -> int;
    auto facility() const noexcept -> int;
    auto identity() const noexcept -> const std::string&;

    auto priorities(std::vector<int> priorities) -> void;

    virtual auto emit(const record_t& record, const string_view& formatted) -> void;
};

}  // namespace sink

template<>
struct factory<sink::syslog_t> {
    static auto type() -> const char*;
    static auto from(const config::node_t& config) -> sink::syslog_t;
};

}  // namespace v1
}  // namespace blackhole
