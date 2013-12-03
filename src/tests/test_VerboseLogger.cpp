#include "Mocks.hpp"

using namespace blackhole;

enum level {
    debug,
    info,
    warning,
    error
};

class base_frontend_t {
public:
    virtual void handle(log::record_t&& record) = 0;
};

template<class Formatter, class Sink>
class frontend_t : public base_frontend_t {
    const std::unique_ptr<Formatter> m_formatter;
    const std::unique_ptr<Sink> m_sink;
public:
    frontend_t(std::unique_ptr<Formatter> formatter, std::unique_ptr<Sink> sink) :
        m_formatter(std::move(formatter)),
        m_sink(std::move(sink))
    {}

    void handle(log::record_t&& record) {
        auto msg = std::move(m_formatter->format(record));
        m_sink->consume(msg);
     }
};

typedef std::function<bool(const log::attributes_t& attributes)> filter_t;

struct default_filter_t {
    static default_filter_t& instance() {
        static default_filter_t filter;
        return filter;
    }

    bool operator()(const log::attributes_t&) {
        return true;
    }

private:
    default_filter_t() {}
};


namespace keyword {

} // namespace keyword

class logger_base_t {
    bool m_enabled;

protected:
    filter_t m_filter;

public:
    logger_base_t() :
        m_enabled(true),
        m_filter(default_filter_t::instance())
    {}

    bool enabled() const {
        return m_enabled;
    }

    void enable() {
        m_enabled = true;
    }

    void disable() {
        m_enabled = false;
    }

    void set_filter(filter_t&& filter) {
        m_filter = filter;
    }
};

template<typename Level>
class verbose_logger_t : public logger_base_t {
    std::unique_ptr<base_frontend_t> m_frontend;
public:
    log::record_t open_record(Level level) const {
        //!@todo: Enabled?
        //!@todo: Create base attributes
        log::attributes_t attributes;
        attributes["severity"] = static_cast<std::uint64_t>(level);
        if (m_filter(attributes)) {
            log::record_t record;
            record.attributes = std::move(attributes);
            return record;
        }
        return log::record_t();
    }

    void push(log::record_t&& record) {
        m_frontend->handle(std::move(record));
    }

    void add_frontend(std::unique_ptr<base_frontend_t> frontend) {
        m_frontend = std::move(frontend);
    }

    template<typename Action>
    struct severity_action_t {
        Action action;
        Level level;

        severity_action_t(Level level) :
            level(level)
        {}

        bool operator()(const log::attributes_t& attributes) const {
            return action(level, static_cast<Level>(boost::get<std::uint64_t>(attributes.at("severity"))));
        }
    };

    struct LessEqThan {
        template<typename L, typename R>
        bool operator()(const L& left, const R& right) const {
            return left <= right;
        }
    };

    class severity_t {
    public:
        filter_t operator>=(Level level) const {
            return severity_action_t<LessEqThan>(level);
        }
    };

    severity_t severity;
};

TEST(verbose_logger_t, Class) {
    verbose_logger_t<level> log;
    UNUSED(log);
}

TEST(logger_base_t, CanBeEnabled) {
    logger_base_t log;
    log.enable();
    EXPECT_TRUE(log.enabled());
}

TEST(logger_base_t, CanBeDisabled) {
    logger_base_t log;
    log.disable();
    EXPECT_FALSE(log.enabled());
}

TEST(logger_base_t, EnabledByDefault) {
    logger_base_t log;
    EXPECT_TRUE(log.enabled());
}

TEST(verbose_logger_t, Manual) {
    verbose_logger_t<level> log;

    //!@note: Factory starts here...
    auto formatter = std::make_unique<formatter::string_t>("[]: %(message)s");
    auto sink = std::make_unique<sink::file_t<>>("/dev/stdout");
    auto frontend = std::make_unique<frontend_t<formatter::string_t, sink::file_t<>>>(std::move(formatter), std::move(sink));
    //!@note ... till here.
    log.add_frontend(std::move(frontend));

    //!@note: Next lines can be hided via macro: LOG(log, debug, "Message %s", "Hell", keyword::answer = 42);
    log::record_t record = log.open_record(level::error);
    if (record.valid()) {
        record.attributes["message"] = utils::format("Some message from: '%s'!", "Hell");
        // Add another attributes.
        log.push(std::move(record));
    }
}

TEST(verbose_logger_t, OpenRecordByDefault) {
    enum level { debug, info, warn, error };
    verbose_logger_t<level> log;
    log::record_t record = log.open_record(level::debug);
    EXPECT_TRUE(record.valid());
}

TEST(verbose_logger_t, OpenRecordForValidVerbosityLevel) {
    enum class level : std::uint64_t { debug, info, warn, error };
    verbose_logger_t<level> log;
    log.set_filter(log.severity >= level::info);
    EXPECT_FALSE(log.open_record(level::debug).valid());
    EXPECT_TRUE(log.open_record(level::info).valid());
    EXPECT_TRUE(log.open_record(level::warn).valid());
    EXPECT_TRUE(log.open_record(level::error).valid());
}

// Allow to make custom filters. severity >= warning || has_tag(urgent) && !!urgent
