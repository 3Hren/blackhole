#pragma once

#include <functional>
#include <string>

#include <boost/asio.hpp>

#include "request/bulk_write.hpp"
#include "log.hpp"
#include "result.hpp"
#include "settings.hpp"
#include "transport.hpp"

namespace elasticsearch {

class client_t {
public:
    typedef boost::asio::io_service loop_type;
    typedef blackhole::synchronized<blackhole::logger_base_t> logger_type;

    typedef std::function<
        void(result_t<response::bulk_write_t>::type&&)
    > callback_type;

private:
    settings_t settings;
    logger_type& log;

    http_transport_t<> transport;

public:
    client_t(const settings_t& settings, loop_type& loop, logger_type& log) :
        settings(settings),
        log(log),
        transport(settings, loop, log)
    {
        transport.add_nodes(settings.endpoints);
        if (settings.sniffer.when.start) {
            ES_LOG(log, "sniff.on.start is true - preparing to update nodes list");
            transport.sniff();
        }
    }

    void cancel() {
        transport.cancel();
    }

    void bulk_write(std::vector<std::string> bulk, callback_type callback) {
        ES_LOG(log, "requesting 'bulk_write' ...");
        transport.perform(
            actions::bulk_write_t(settings.index, settings.type, std::move(bulk)),
            callback
        );
    }
};

} // namespace elasticsearch
