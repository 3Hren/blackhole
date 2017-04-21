#pragma once

#include <string>
#include <vector>

#include "blackhole/sink.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

class syslog_t : public sink_t {
    struct {
        int option;
        int facility;
        std::string identity;
        std::vector<int> priorities;
    } data;

public:
    syslog_t();
    syslog_t(const syslog_t& other) = delete;
    syslog_t(syslog_t&& other) = default;
    ~syslog_t();

    auto operator=(const syslog_t& other) -> syslog_t& = delete;
    auto operator=(syslog_t&& other) -> syslog_t& = default;

    auto option() const noexcept -> int;
    auto facility() const noexcept -> int;
    auto identity() const noexcept -> const std::string&;
    auto priorities() const -> std::vector<int>;
    auto priorities(std::vector<int> priorities) -> void;

    auto emit(const record_t& record, const string_view& formatted) -> void override;
};

}  // namespace sink
}  // namespace v1
}  // namespace blackhole
