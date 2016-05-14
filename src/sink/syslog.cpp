#include "blackhole/sink/syslog.hpp"

#include <syslog.h>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/procname.hpp"
#include "blackhole/detail/sink/syslog.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

syslog_t::syslog_t() {
   data.option = LOG_PID;
   data.facility = LOG_USER;
   data.identity = detail::procname().to_string();


   ::openlog(identity().c_str(), option(), facility());
}

syslog_t::~syslog_t() {
    ::closelog();
}

auto syslog_t::option() const noexcept -> int {
    return data.option;
}

auto syslog_t::facility() const noexcept -> int {
    return data.facility;
}

auto syslog_t::identity() const noexcept -> const std::string& {
    return data.identity;
}

auto syslog_t::priorities(std::vector<int> priorities) -> void {
    data.priorities = std::move(priorities);
}

auto syslog_t::emit(const record_t& record, const string_view& formatted) -> void {
    const auto severity = static_cast<std::size_t>(record.severity());

    int priority;
    if (severity < data.priorities.size()) {
        priority = data.priorities[severity];
    } else {
        priority = LOG_ERR;
    }

    ::syslog(priority, "%.*s", static_cast<int>(formatted.size()), formatted.data());
}

}  // namespace sink

namespace experimental {

auto factory<sink::syslog_t>::type() const noexcept -> const char* {
    return "syslog";
}

auto factory<sink::syslog_t>::from(const config::node_t& config) const -> std::unique_ptr<sink_t> {
    sink::syslog_t syslog;

    if (auto mapping = config["priorities"]) {
        std::vector<int> priorities;
        mapping.each([&](const config::node_t& config) {
            priorities.emplace_back(config.to_sint64());
        });

        syslog.priorities(std::move(priorities));
    }

    return std::unique_ptr<sink_t>(new sink::syslog_t(std::move(syslog)));
}

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
