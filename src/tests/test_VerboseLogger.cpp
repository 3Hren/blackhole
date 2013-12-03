#include "Mocks.hpp"

#include <ctime>

using namespace blackhole;

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


struct LessEqThan {
    template<typename L, typename R>
    bool operator()(const L& left, const R& right) const {
        return left <= right;
    }
};

log::attributes_t merge(const std::initializer_list<log::attributes_t>& args) {
    log::attributes_t summary;
    for (auto it = args.begin(); it != args.end(); ++it) {
        summary.insert(it->begin(), it->end());
    }
    return summary;
}


namespace keyword {

template<typename T>
struct severity_t {
    static_assert(std::is_enum<T>::value, "severity type must be enum");

    typedef typename std::underlying_type<T>::type type;

    static const char* name() {
        return "severity";
    }

    filter_t operator>=(T value) const {
        return action_t<LessEqThan>(value);
    }

    log::attributes_t operator=(T value) const {
        log::attributes_t attributes;
        attributes[name()] = static_cast<type>(value);
        return attributes;
    }

    template<typename Action>
    struct action_t {
        Action action;
        T value;

        action_t(T value) :
            value(value)
        {}

        bool operator()(const log::attributes_t& attributes) const {
            //!@todo: attributes.extract<keyword::severity>();
            return action(value, static_cast<T>(boost::get<type>(attributes.at(severity_t<T>::name()))));
        }
    };
};

template<typename T>
static severity_t<T>& severity() {
    static severity_t<T> self;
    return self;
}

// DECLARE_KEYWORD(severity, level);

} // namespace keyword

class logger_base_t {
    bool m_enabled;

protected:
    filter_t m_filter;
    std::unique_ptr<base_frontend_t> m_frontend;

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

    void add_frontend(std::unique_ptr<base_frontend_t> frontend) {
        m_frontend = std::move(frontend);
    }

    log::record_t open_record() const {
        return open_record(log::attributes_t());
    }

    log::record_t open_record(log::attributes_t&& local_attributes) const {
        if (enabled()) {
            log::attributes_t attributes = merge({
                std::move(get_scoped_attributes()),
                std::move(local_attributes)
            });

            if (m_filter(attributes)) {
                log::record_t record;
                record.attributes = std::move(attributes);
                return record;
            }
        }

        return log::record_t();
    }

    void push(log::record_t&& record) {
        m_frontend->handle(std::move(record));
    }

private:
    log::attributes_t get_scoped_attributes() const {
        log::attributes_t attributes;
        attributes["timestamp_id"] = std::time(nullptr);
        return attributes;
    }
};

template<typename Level>
class verbose_logger_t : public logger_base_t {
    typedef typename std::underlying_type<Level>::type level_type;

public:
    log::record_t open_record(Level level) const {
        return logger_base_t::open_record(keyword::severity<Level>() = level);
    }
};

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

TEST(logger_base_t, OpenRecordByDefault) {
    logger_base_t log;
    EXPECT_TRUE(log.open_record().valid());
}

TEST(logger_base_t, DoNotOpenRecordIfDisabled) {
    logger_base_t log;
    log.disable();
    EXPECT_FALSE(log.open_record().valid());
}

TEST(verbose_logger_t, Class) {
    enum level : std::uint64_t { debug, info, warn, error };
    verbose_logger_t<level> log;
    UNUSED(log);
}

TEST(verbose_logger_t, OpenRecordByDefault) {
    enum level : std::uint64_t { debug, info, warn, error };
    verbose_logger_t<level> log;
    log::record_t record = log.open_record(level::debug);
    EXPECT_TRUE(record.valid());
}

TEST(verbose_logger_t, OpenRecordForValidVerbosityLevel) {
    enum class level : std::uint64_t { debug, info, warn, error };
    verbose_logger_t<level> log;
    log.set_filter(keyword::severity<level>() >= level::info);
    EXPECT_FALSE(log.open_record(level::debug).valid());
    EXPECT_TRUE(log.open_record(level::info).valid());
    EXPECT_TRUE(log.open_record(level::warn).valid());
    EXPECT_TRUE(log.open_record(level::error).valid());
}

// Allow to make custom filters. severity >= warning || has_tag(urgent) && !!urgent

TEST(verbose_logger_t, Manual) {
    enum level : std::uint64_t { debug, info, warn, error };
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
