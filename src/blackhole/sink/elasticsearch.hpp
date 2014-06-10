#pragma once

#include <queue>

#include <boost/asio.hpp>
#include <boost/thread/barrier.hpp>

#include <blackhole/formatter/string.hpp>
#include <blackhole/log.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/sink/stream.hpp>
#include <blackhole/synchronized.hpp>
#include <blackhole/utils/atomic.hpp>
#include <blackhole/utils/format.hpp>

#include "elasticsearch/client.hpp"
#include "elasticsearch/result.hpp"
#include "elasticsearch/settings.hpp"
#include "elasticsearch/queue.hpp"

namespace blackhole {

namespace sink {

namespace elasticsearch_ {

struct config_t {
    std::uint16_t bulk;
    std::uint32_t interval;
    elasticsearch::settings_t settings;

    config_t() :
        bulk(100),
        interval(1000)
    {}
};

} // namespace elasticsearch

class elasticsearch_t {
public:
    typedef elasticsearch_::config_t config_type;

private:
    synchronized<logger_base_t> log;

    boost::posix_time::milliseconds interval;

    std::atomic<bool> stopped;
    boost::asio::io_service loop;
    boost::asio::deadline_timer timer;

    std::thread thread;
    bulk::queue_t<std::string> queue;
    elasticsearch::client_t client;

public:
    elasticsearch_t(std::uint16_t bulk = 100,
                    std::uint32_t interval = 1000,
                    elasticsearch::settings_t settings = elasticsearch::settings_t()) :
        log(elasticsearch::logger_factory_t::create()),
        interval(interval),
        stopped(true),
        timer(loop),
        thread(start()),
        queue(bulk, std::bind(&elasticsearch_t::on_bulk, this, std::placeholders::_1)),
        client(settings, loop, log)
    {}

    elasticsearch_t(const config_type& config) :
        log(elasticsearch::logger_factory_t::create()),
        interval(config.interval),
        stopped(true),
        timer(loop),
        thread(start()),
        queue(config.bulk, std::bind(&elasticsearch_t::on_bulk, this, std::placeholders::_1)),
        client(config.settings, loop, log)
    {}

    ~elasticsearch_t() {
        stop();
    }

    static const char* name() {
        return "elasticsearch";
    }

    void consume(std::string message) {
        if (stopped) {
            ES_LOG(log, "dropping '%s', because worker thread is stopped", message);
            return;
        }

        ES_LOG(log, "pushing '%s' to the queue", message);
        queue.push(std::move(message));
    }

private:
    std::thread start() {
        boost::barrier started(2);
        std::thread thread(
            std::bind(&elasticsearch_t::run, this, std::ref(started))
        );
        started.wait();
        return std::move(thread);
    }

    void run(boost::barrier& started) {
        BOOST_ASSERT(stopped);

        ES_LOG(log, "starting elasticsearch sink ...");
        loop.post(
            std::bind(&elasticsearch_t::on_started, this, std::ref(started))
        );

        timer.expires_from_now(interval);
        timer.async_wait(
            std::bind(&elasticsearch_t::on_timer, this, std::placeholders::_1)
        );
        boost::system::error_code ec;
        loop.run(ec);
        ES_LOG(log, "elasticsearch sink has been stopped: %s", ec ? ec.message() : "ok");
    }

    void stop() {
        stopped = true;
        timer.cancel();
        client.stop();
        loop.stop();

        if (thread.joinable()) {
            ES_LOG(log, "stopping worker thread ...");
            thread.join();
        }
    }

    void on_started(boost::barrier& started) {
        ES_LOG(log, "elasticsearch sink has been started");
        stopped = false;
        started.wait();
    }

    void on_bulk(std::vector<std::string>&& result) {
        loop.post(
            std::bind(&elasticsearch_t::handle_bulk, this, std::move(result))
        );
    }

    void handle_bulk(std::vector<std::string> result) {
        ES_LOG(log, "processing bulk event ...");
        process(std::move(result));
    }

    void on_timer(const boost::system::error_code& ec) {
        if (ec) {
            ES_LOG(log, "processing timer event failed: %s", ec.message());
        } else {
            ES_LOG(log, "processing timer event ...");
            process(std::move(queue.dump()));
        }
    }

    void process(std::vector<std::string>&& result) {
        if (stopped) {
            ES_LOG(log, "dropping %d messages, because worker thread is stopped",
                   result.size());
            return;
        }

        ES_LOG(log, "processing bulk containing %d messages ...", result.size());
        client.bulk_write(
            std::move(result),
            std::bind(&elasticsearch_t::on_response, this, std::placeholders::_1)
        );

        timer.expires_from_now(interval);
        timer.async_wait(
            std::bind(&elasticsearch_t::on_timer, this, std::placeholders::_1)
        );
    }

    void on_response(elasticsearch::result_t<
                        elasticsearch::response::bulk_write_t
                     >::type&& result) {
        if (boost::get<elasticsearch::error_t>(&result)) {
            ES_LOG(log, "bulk write failed");
            return;
        }

        ES_LOG(log, "success!");
    }
};

} // namespace sink

template<>
struct factory_traits<sink::elasticsearch_t> {
    typedef sink::elasticsearch_t sink_type;
    typedef sink_type::config_type config_type;

    static
    void
    map_config(const aux::extractor<sink_type>& ex, config_type& config) {
        ex["bulk"].to(config.bulk);
        ex["interval"].to(config.interval);
        ex["index"].to(config.settings.index);
        ex["type"].to(config.settings.type);
        auto endpoints = ex["endpoints"].get<std::vector<std::string>>();
        //!@todo: Map vector of strings to the real endpoints collection.

        ex["sniffer"]["when"]["start"].to(config.settings.sniffer.when.start);
        ex["sniffer"]["when"]["error"].to(config.settings.sniffer.when.error);
        ex["sniffer"]["interval"].to(config.settings.sniffer.invertal);
        ex["connections"].to(config.settings.connections);
        ex["retries"].to(config.settings.retries);
        ex["timeout"].to(config.settings.timeout);
    }
};

} // namespace blackhole
