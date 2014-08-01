#pragma once

#include <queue>

#include <boost/asio.hpp>
#include <boost/thread/barrier.hpp>

#include <blackhole/detail/logger/pusher.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/sink/stream.hpp>
#include <blackhole/sink/thread.hpp>
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
    std::uint8_t workers;
    elasticsearch::settings_t settings;

    config_t() :
        bulk(100),
        interval(1000),
        workers(1)
    {}
};

} // namespace elasticsearch

class worker_t {
public:
    typedef synchronized<logger_base_t> logger_type;
    typedef boost::asio::io_service loop_type;
    typedef elasticsearch::result_t<
        elasticsearch::response::bulk_write_t
    >::type result_type;
    typedef std::function<void(result_type&&)> callback_type;

private:
    logger_type& log;

    loop_type loop;
    std::unique_ptr<loop_type::work> work;
    elasticsearch::client_t client;
    std::atomic<bool> stopped;
    std::thread thread;

public:
    worker_t(elasticsearch::settings_t settings, logger_type& log) :
        log(log),
        work(utils::make_unique<loop_type::work>(loop)),
        client(settings, loop, log),
        stopped(true),
        thread(&worker_t::run, this)
    {}

    ~worker_t() {
        stopped = true;
        ES_LOG(log, "stopping worker thread ...");
        work.reset();
        client.cancel();
        thread.join();
    }

    void handle(std::vector<std::string>&& bulk, callback_type callback) {
        if (stopped) {
            ES_LOG(log, "drop %d messages, because worker is stopped", bulk.size());
            return;
        }

        loop.post(
            std::bind(
                &elasticsearch::client_t::bulk_write,
                &client,
                std::move(bulk),
                callback
            )
        );
    }

private:
    void run() {
        ES_LOG(log, "starting worker thread ...");
        loop.post(std::bind(&worker_t::on_started, this));
        boost::system::error_code ec;
        loop.run(ec);
        ES_LOG(log, "worker thread has been stopped: %s", ec ? ec.message() : "ok");
    }

    void on_started() {
        stopped = false;
        ES_LOG(log, "worker thread has been started");
    }
};

class worker_pool_t {
public:
    typedef worker_t::logger_type logger_type;
    typedef worker_t::callback_type callback_type;

private:
    std::vector<std::unique_ptr<worker_t>> workers;
    std::vector<std::unique_ptr<worker_t>>::size_type current;

    mutable std::mutex mutex;

public:
    worker_pool_t(std::uint8_t count,
                  elasticsearch::settings_t settings,
                  logger_type& log) :
        current(0)
    {
        for (std::uint8_t i = 0; i < count; ++i) {
            workers.emplace_back(utils::make_unique<worker_t>(settings, log));
        }
    }

    ~worker_pool_t() {
        std::lock_guard<std::mutex> lock(mutex);
        workers.clear();
    }

    void handle(std::vector<std::string>&& bulk, callback_type callback) {
        std::lock_guard<std::mutex> lock(mutex);
        if (workers.empty()) {
            return;
        }

        if (current >= workers.size()) {
            current = 0;
        }

        workers.at(current++)->handle(std::move(bulk), callback);
    }
};

class elasticsearch_t {
public:
    typedef elasticsearch_::config_t config_type;

private:
    synchronized<logger_base_t> log;

    boost::posix_time::milliseconds interval;

    boost::asio::io_service loop;
    boost::asio::deadline_timer timer;
    std::thread thread;
    bulk::queue_t<std::string> queue;
    std::unique_ptr<worker_pool_t> pool;

    mutable std::mutex mutex;

public:
    elasticsearch_t(std::uint16_t bulk = 10,
                    std::uint32_t interval = 1000,
                    std::uint8_t workers = 1,
                    elasticsearch::settings_t settings = elasticsearch::settings_t()) :
        log(elasticsearch::logger_factory_t::create()),
        interval(interval),
        timer(loop),
        thread(start()),
        queue(bulk, std::bind(&elasticsearch_t::on_bulk, this, std::placeholders::_1)),
        pool(utils::make_unique<worker_pool_t>(workers, settings, log))
    {}

    elasticsearch_t(const elasticsearch_::config_t& config) :
        log(elasticsearch::logger_factory_t::create()),
        interval(config.interval),
        timer(loop),
        thread(start()),
        queue(config.bulk, std::bind(&elasticsearch_t::on_bulk, this, std::placeholders::_1)),
        pool(utils::make_unique<worker_pool_t>(config.workers, config.settings, log))
    {}

    ~elasticsearch_t() {
        stop();
    }

    static const char* name() {
        return "elasticsearch";
    }

    void consume(std::string message) {
        ES_LOG(log, "pushing '%s' to the queue", message);
        queue.push(std::move(message));
    }

private:
    std::thread start() {
        std::thread thread(
            std::bind(&elasticsearch_t::run, this)
        );
        return std::move(thread);
    }

    void run() {
        ES_LOG(log, "starting elasticsearch sink thread ...");
        loop.post(
            std::bind(&elasticsearch_t::on_started, this)
        );

        timer.expires_from_now(interval);
        timer.async_wait(
            std::bind(&elasticsearch_t::on_timer, this, std::placeholders::_1)
        );
        boost::system::error_code ec;
        loop.run(ec);
        ES_LOG(log, "elasticsearch sink thread has been stopped: %s",
               ec ? ec.message() : "ok");
    }

    void stop() {
        ES_LOG(log, "stopping elasticsearch sink thread ...");
        std::unique_lock<std::mutex> lock(mutex);
        pool.reset();
        lock.unlock();

        timer.cancel();
        if (thread.joinable()) {
            thread.join();
        }
    }

    void on_started() {
        ES_LOG(log, "elasticsearch sink thread has been started");
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
        ES_LOG(log, "processing bulk containing %d messages ...", result.size());
        std::unique_lock<std::mutex> lock(mutex);
        if (!pool) {
            ES_LOG(log, "dropping %d messages, because worker thread is stopped", result.size());
            return;
        }
        pool->handle(
            std::move(result),
            std::bind(&elasticsearch_t::on_response, this, std::placeholders::_1)
        );
        lock.unlock();

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

template<>
struct thread_safety<elasticsearch_t> :
    public std::integral_constant<
        thread::safety_t,
        thread::safety_t::safe
    >::type
{};

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
        ex["workers"].to(config.workers);
        ex["index"].to(config.settings.index);
        ex["type"].to(config.settings.type);

        auto endpoints = ex["endpoints"].get<dynamic_t::array_t>();
        boost::asio::io_service loop;
        for (auto it = endpoints.begin(); it != endpoints.end(); ++it) {
            config.settings.endpoints.push_back(
                elasticsearch::resolver<
                    boost::asio::ip::tcp
                >::resolve(it->to<std::string>(), loop)
            );
        }

        ex["sniffer"]["when"]["start"].to(config.settings.sniffer.when.start);
        ex["sniffer"]["when"]["error"].to(config.settings.sniffer.when.error);
        ex["sniffer"]["interval"].to(config.settings.sniffer.invertal);
        ex["connections"].to(config.settings.connections);
        ex["retries"].to(config.settings.retries);
        ex["timeout"].to(config.settings.timeout);
    }
};

} // namespace blackhole
