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

namespace syslog {

backend_t::~backend_t() = default;

namespace {

class native_t : public backend_t {
public:
    native_t(const char* identity, int option = LOG_PID, int facility = LOG_USER) {
        ::openlog(identity, option, facility);
    }

    ~native_t() {
        ::closelog();
    }
};

}  // namespace
}  // namespace syslog

class syslog_t : public sink_t {
    std::unique_ptr<syslog::backend_t> backend;
    syslog::priority_t priority;

public:
    syslog_t(std::unique_ptr<syslog::backend_t> backend, syslog::priority_t priority) :
        backend(std::move(backend)),
        priority(std::move(priority))
    {}

    auto emit(const record_t& record, const string_view& message) -> void override {
    }
};

// auto priority(severity_t severity) const noexcept -> int {
//     if (severity < priorities.size()) {
//         return priorities[severity];
//     } else {
//         return LOG_ERR;
//     }
// }

// syslog_t::syslog_t(std::unique_ptr<syslog::backend_t> backend) :
//     backend(std::move(backend))
// {
//     // this->backend->open();
// }
//
// syslog_t::~syslog_t() {
//     // backend->close();
// }

// auto syslog_t::emit(const record_t& record, const string_view& formatted) -> void {
//     // ::syslog(priority(record.severity()), "%.*s", static_cast<int>(formatted.size()), formatted.data());
// }

}  // namespace sink

auto factory<sink::syslog_t>::type() -> const char* {
    return "syslog";
}

auto factory<sink::syslog_t>::from(const config::node_t& config) -> std::unique_ptr<sink::syslog_t> {
    // sink::syslog_t syslog;

    // if (auto mapping = config["priorities"]) {
    //     std::vector<int> priorities;
    //     mapping.each([&](const config::node_t& config) {
    //         priorities.emplace_back(config.to_sint64());
    //     });
    //
    //     syslog.priorities(std::move(priorities));
    // }

    // return std::unique_ptr<sink::syslog_t>(new sink::syslog_t(std::move(syslog)));
}

auto factory<sink::syslog_t>::construct(std::unique_ptr<backend_t> backend, priority_t priority) ->
    std::unique_ptr<syslog_t>
{
    // return
}

}  // namespace v1
}  // namespace blackhole
