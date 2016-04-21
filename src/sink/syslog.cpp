#include "blackhole/sink/syslog.hpp"

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/procname.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

class syslog_t::inner_t {
public:
    int option;
    int facility;
    std::string identity;
    std::vector<int> priorities;
};

syslog_t::syslog_t() :
    inner(new inner_t)
{
    inner->option = LOG_PID;
    inner->facility = LOG_USER;
    inner->identity = detail::procname().to_string();


    ::openlog(inner->identity.c_str(), inner->option, inner->facility);
}

syslog_t::syslog_t(syslog_t&& other) = default;

syslog_t::~syslog_t() {
    ::closelog();
}

auto syslog_t::option() const noexcept -> int {
    return inner->option;
}

auto syslog_t::facility() const noexcept -> int {
    return inner->facility;
}

auto syslog_t::identity() const noexcept -> const std::string& {
    return inner->identity;
}

auto syslog_t::priorities(std::vector<int> priorities) -> void {
    inner->priorities = std::move(priorities);
}

auto syslog_t::emit(const record_t& record, const string_view& formatted) -> void {
    const auto severity = static_cast<std::size_t>(record.severity());

    int priority;
    if (severity < inner->priorities.size()) {
        priority = inner->priorities[severity];
    } else {
        priority = LOG_ERR;
    }

    ::syslog(priority, "%.*s", static_cast<int>(formatted.size()), formatted.data());
}

}  // namespace sink

auto factory<sink::syslog_t>::type() -> const char* {
    return "syslog";
}

auto factory<sink::syslog_t>::from(const config::node_t& config) -> sink::syslog_t {
    sink::syslog_t syslog;

    if (auto mapping = config["priorities"]) {
        std::vector<int> priorities;
        mapping.each([&](const config::node_t& config) {
            priorities.emplace_back(config.to_sint64());
        });

        syslog.priorities(std::move(priorities));
    }

    return syslog;
}

}  // namespace v1
}  // namespace blackhole
