#include "essentials.hpp"

#include "blackhole/filter/severity.hpp"
#include "blackhole/formatter/string.hpp"
#include "blackhole/handler/blocking.hpp"
#include "blackhole/handler/dev.hpp"
#include "blackhole/registry.hpp"
#include "blackhole/sink/asynchronous.hpp"
#include "blackhole/sink/console.hpp"
#include "blackhole/sink/file.hpp"
#include "blackhole/sink/null.hpp"
#include "blackhole/sink/socket/tcp.hpp"
#include "blackhole/sink/socket/udp.hpp"
#include "blackhole/sink/syslog.hpp"

namespace blackhole {
inline namespace v1 {

auto essentials(registry_t& registry) -> void {
    registry.add<filter::severity_t>();

    registry.add<formatter::string_t>();

    registry.add<sink::asynchronous_t>(registry);
    registry.add<sink::console_t>(registry);
    registry.add<sink::file_t>(registry);
    registry.add<sink::null_t>();
    registry.add<sink::socket::tcp_t>(registry);
    registry.add<sink::socket::udp_t>(registry);
    registry.add<sink::syslog_t>(registry);

    registry.add<handler::blocking_t>(registry);
    registry.add<handler::dev_t>(registry);
}

}  // namespace v1
}  // namespace blackhole
