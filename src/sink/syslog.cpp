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

    virtual auto open() -> void override {
        ::openlog(identity, option, facility);
    }

    virtual auto close() noexcept -> void override {
        ::closelog();
    }
};

class default_t : public syslog_t {
    std::unique_ptr<syslog::backend_t> backend;
    syslog::priority_map priomap;

public:
    default_t(std::unique_ptr<syslog::backend_t> backend, syslog::priority_map priomap) :
        backend(std::move(backend)),
        priomap(std::move(priomap))
    {
        this->backend->open();
    }

    ~default_t() {
        backend->close();
    }

    virtual auto option() const -> int override {
        return 0; //backend->option();
    }

    virtual auto facility() const -> int override {
        return 0; // backend->facility();
    }

    virtual auto identity() const -> const char* override {
        return nullptr;
    }

    virtual auto priority(severity_t severity) const -> int override {
        return 0;
    }

    virtual auto emit(const record_t& record, const string_view& message) -> void override {
        backend->write(priority(record.severity()), message);
    }
};

}  // namespace
}  // namespace syslog

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

auto factory<sink::syslog_t>::type() const noexcept -> const char* {
    return "syslog";
}

auto factory<sink::syslog_t>::from(const config::node_t& config) const -> std::unique_ptr<sink_t> {
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

auto factory<sink::syslog_t>::construct(std::unique_ptr<backend_t> backend, priority_map priomap) ->
    std::unique_ptr<syslog_t>
{
    // return
}

}  // namespace v1
}  // namespace blackhole
