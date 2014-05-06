#include <queue>

#include <boost/asio.hpp>

#include <blackhole/formatter/string.hpp>
#include <blackhole/log.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/sink/stream.hpp>
#include <blackhole/synchronized.hpp>
#include <blackhole/utils/atomic.hpp>
#include <blackhole/utils/format.hpp>

#include "../global.hpp"

#define LOG(__log__, ...) \
    if (blackhole::log::record_t record = __log__.open_record()) \
        blackhole::aux::make_scoped_pump(__log__, record, __VA_ARGS__)

namespace blackhole {

namespace sink {

namespace bulk {

template<typename T>
class queue_t {
public:
    typedef T value_type;
    typedef std::vector<value_type> bulk_type;
    typedef std::function<void(bulk_type&&)> callback_type;

private:
    const std::uint16_t bulk;
    callback_type callback;

    std::queue<T> queue;
    mutable std::mutex mutex;
public:
    queue_t(std::uint16_t bulk, callback_type callback) :
        bulk(bulk),
        callback(callback)
    {}

    void push(T&& value) {
        std::unique_lock<std::mutex> lock(mutex);

        queue.push(std::move(value));
        if (queue.size() >= bulk) {
            auto result = dump(lock);
            lock.unlock();
            callback(std::move(result));
        }
    }

    std::vector<std::string> dump() {
        std::lock_guard<std::mutex> lock(mutex);
        return dump(lock);
    }

private:
    template<class Lock>
    std::vector<std::string> dump(Lock&) {
        std::vector<std::string> result;
        result.reserve(bulk);
        for (uint i = 0; i < bulk && !queue.empty(); ++i) {
            result.push_back(std::move(queue.front()));
            queue.pop();
        }
        return result;
    }
};

} // namespace bulk

namespace elasticsearch {

//!@todo: Dummy yet.
struct config_t {
};

struct settings_t {
    typedef boost::asio::ip::tcp::endpoint endpoint_type;

    std::string index;
    std::vector<endpoint_type> endpoints;
    struct {
        struct {
            bool start;
        } when;
    } sniffer;

    settings_t() :
        index("log"),
        endpoints(std::vector<endpoint_type>({
            { endpoint_type(boost::asio::ip::address_v4(), 9200) }
        })),
        sniffer({ { true } })
    {}
};

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

class logger_factory_t {
public:
    static logger_base_t create() {
        logger_base_t logger;
        auto formatter = utils::make_unique<
            formatter::string_t
        >("[%(timestamp)s]: %(message)s");

        auto sink = utils::make_unique<
            sink::stream_t
        >(sink::stream_t::output_t::stdout);

        auto frontend = utils::make_unique<
            frontend_t<
                formatter::string_t,
                sink::stream_t
            >
        >(std::move(formatter), std::move(sink));

        logger.add_frontend(std::move(frontend));
        return logger;
    }
};

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
        log(logger_factory_t::create()),
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
            message,
            std::bind(&elasticsearch_t::on_success, this),
            std::bind(&elasticsearch_t::on_error, this, std::placeholders::_1)
        );

        timer.expires_from_now(interval);
        timer.async_wait(
            std::bind(&elasticsearch_t::on_timer, this, std::placeholders::_1)
        );
    }

    void on_success() {
        LOG(log, "success!");
    }

    void on_error(const boost::system::error_code& ec) {
        LOG(log, "ec: %s", ec.message());
    }
};

} // namespace sink

} // namespace blackhole

using namespace blackhole;
using blackhole::sink::elasticsearch_t;

TEST(elasticsearch_t, Class) {
    elasticsearch_t sink;
    UNUSED(sink);
}

TEST(elasticsearch_t, Manual) {
    elasticsearch_t sink;
    for (int i = 0; i < 200; ++i) {
        sink.consume((boost::format("{\"key\":%d}") % i).str());
    }
}
