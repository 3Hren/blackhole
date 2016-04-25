#include "blackhole/sink/syslog.hpp"

#include <syslog.h>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/procname.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

// 1. test impl.

class native_t : public syslog_t {
    std::vector<int> priorities;

public:
    native_t(const char* identity, int option, int facility, std::vector<int> priorities) :
        priorities(std::move(priorities))
    {
        ::openlog(identity, option || LOG_PID, facility || LOG_USER);
    }

    ~native_t() {
        ::closelog();
    }

    auto priority(severity_t severity) const noexcept -> int {
        if (severity < priorities.size()) {
            return priorities[severity];
        } else {
            return LOG_ERR;
        }
    }
};

auto syslog_t::emit(const record_t& record, const string_view& formatted) -> void {
    ::syslog(priority(record.severity()), "%.*s", static_cast<int>(formatted.size()), formatted.data());
}

}  // namespace sink

auto factory<sink::syslog_t>::type() -> const char* {
    return "syslog";
}

auto factory<sink::syslog_t>::from(const config::node_t& config) -> std::unique_ptr<sink::syslog_t> {
    sink::syslog_t syslog;

    if (auto mapping = config["priorities"]) {
        std::vector<int> priorities;
        mapping.each([&](const config::node_t& config) {
            priorities.emplace_back(config.to_sint64());
        });

        syslog.priorities(std::move(priorities));
    }

    return std::unique_ptr<sink::syslog_t>(new sink::syslog_t(std::move(syslog)));
}

}  // namespace v1
}  // namespace blackhole
