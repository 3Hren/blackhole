#pragma once

#include <functional>
#include <string>

#include <boost/asio.hpp>

#include "log.hpp"
#include "settings.hpp"
#include "transport.hpp"

using blackhole::logger_base_t;
using blackhole::synchronized;

namespace elasticsearch {

class client_t {
public:
    typedef boost::asio::io_service loop_type;
    typedef synchronized<logger_base_t> logger_type;

    typedef std::function<void()> callback_type;
    typedef std::function<
        void(const boost::system::error_code&)
    > errback_type;

private:
    settings_t settings;
    loop_type& loop;
    logger_type& log;

    http_transport_t transport;

public:
    client_t(const settings_t& settings, loop_type& loop, logger_type& log) :
        settings(settings),
        loop(loop),
        log(log)
    {
//        transport.add_nodes(settings.endpoints);
        if (settings.sniffer.when.start) {
            LOG(log, "sniff.when.start is true - preparing to update nodes list");
//            transport.sniff();
        }
    }

    void bulk_write(std::string message,
                    callback_type callback,
                    errback_type errback) {
    }
};

} // namespace elasticsearch
