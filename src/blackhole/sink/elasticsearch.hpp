#pragma once

#include <queue>

#include <boost/asio.hpp>

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

namespace es {

//!@todo: Dummy yet.
struct config_t {};

} // namespace es

namespace sink {

class elasticsearch_t {
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

    //!@todo: Config constructor.

    ~elasticsearch_t() {
        stop();
    }

    void consume(std::string&& message) {
        if (stopped) {
            LOG(log, "dropping '%s', because worker thread is stopped", message);
            return;
        }

        LOG(log, "pushing '%s' to the queue", message);
        queue.push(std::move(message));
    }

private:
    std::thread start() {
        std::mutex mutex;
        std::unique_lock<std::mutex> lock(mutex);
        std::condition_variable started;
        std::thread thread(
            std::bind(&elasticsearch_t::run, this, std::ref(started))
        );
        started.wait(lock);
        return std::move(thread);
    }

    void run(std::condition_variable& started) {
        BOOST_ASSERT(stopped);

        LOG(log, "starting elasticsearch sink ...");
        loop.post(
            std::bind(&elasticsearch_t::on_started, this, std::ref(started))
        );
        timer.expires_from_now(interval);
        timer.async_wait(
            std::bind(&elasticsearch_t::on_timer, this, std::placeholders::_1)
        );
        boost::system::error_code ec;
        loop.run(ec);
        LOG(log, "elasticsearch sink has been stopped: %s", ec ? ec.message() : "ok");
    }

    void stop() {
        loop.stop();
        stopped = true;

        if (thread.joinable()) {
            LOG(log, "stopping worker thread ...");
            thread.join();
        }
    }

    void on_started(std::condition_variable& started) {
        LOG(log, "elasticsearch sink has been started");
        stopped = false;
        started.notify_all();
    }

    void on_bulk(std::vector<std::string>&& result) {
        loop.post(
            std::bind(&elasticsearch_t::handle_bulk, this, std::move(result))
        );
    }

    void handle_bulk(std::vector<std::string> result) {
        LOG(log, "processing bulk event ...");
        process(std::move(result));
    }

    void on_timer(const boost::system::error_code& ec) {
        if (ec) {
            LOG(log, "processing timer event failed: %s", ec.message());
        } else {
            LOG(log, "processing timer event ...");
            process(std::move(queue.dump()));
        }
    }

    void process(std::vector<std::string>&& result) {
        LOG(log, "processing bulk containing %d messages ...", result.size());
        std::string message = boost::algorithm::join(result, "");
        client.bulk_write(
            std::move(message),
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
            LOG(log, "bulk write failed");
            return;
        }

        LOG(log, "success!");
    }
};

} // namespace sink

} // namespace blackhole
