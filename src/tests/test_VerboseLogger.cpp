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

namespace helper {

struct LessEqThan {
    template<typename L, typename R>
    static bool execute(const L& left, const R& right) {
        return left <= right;
    }
};

struct Eq {
    template<typename L, typename R>
    static bool execute(const L& left, const R& right) {
        return left == right;
    }
};

} // namespace helper

log::attributes_t merge(const std::initializer_list<log::attributes_t>& args) {
    log::attributes_t summary;
    for (auto it = args.begin(); it != args.end(); ++it) {
        summary.insert(it->begin(), it->end());
    }

    return summary;
}

namespace expr {

template<typename T>
struct has_attr_action_t {
    bool operator()(const log::attributes_t& attributes) const {
        return attributes.find(T::name()) != attributes.end();
    }
};

template<typename T>
has_attr_action_t<T> has_attr(const T&) {
    return has_attr_action_t<T>();
}

} // namespace expr

namespace keyword {

template<typename T, class = void>
struct traits {
    static inline void pack(log::attributes_t& attributes, const std::string& name, const T& value) {
        attributes[name] = value;
    }

    static inline T pack(const T& value) {
        return value;
    }

    static inline T extract(const log::attributes_t& attributes, const std::string& name) {
        return boost::get<T>(attributes.at(name));
    }
};

template<typename T>
struct traits<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    typedef typename std::underlying_type<T>::type underlying_type;

    static inline void pack(log::attributes_t& attributes, const std::string& name, const T& value) {
        attributes[name] = static_cast<underlying_type>(value);
    }

    static inline underlying_type pack(const T& value) {
        return static_cast<underlying_type>(value);
    }

    static inline T extract(const log::attributes_t& attributes, const std::string& name) {
        return static_cast<T>(boost::get<underlying_type>(attributes.at(name)));
    }
};

//!@todo: Need testing.
template<typename T, typename NameProvider>
struct keyword_t {
    static const char* name() {
        return NameProvider::name();
    }

    log::attribute_pair_t operator =(T value) const {
        return std::make_pair(name(), traits<T>::pack(value));
    }

    filter_t operator >=(T value) const {
        return action_t<helper::LessEqThan>({ value });
    }

    filter_t operator ==(T value) const {
        return action_t<helper::Eq>({ value });
    }

    template<typename Action>
    struct action_t {
        T value;

        bool operator()(const log::attributes_t& attributes) const {
            return Action::execute(value, traits<T>::extract(attributes, name()));
        }
    };
};

namespace tag {

struct severity_t {
    static const char* name() { return "severity"; }
};

} // namespace tag

template<typename T>
static keyword_t<T, tag::severity_t>& severity() {
    static keyword_t<T, tag::severity_t> self;
    return self;
}

#define DECLARE_KEYWORD(Name, T) \
    namespace tag { \
        struct Name##_t { \
            static const char* name() { return #Name; } \
        }; \
    } \
    static keyword_t<T, tag::Name##_t>& Name() { \
        static keyword_t<T, tag::Name##_t> self; \
        return self; \
    }

DECLARE_KEYWORD(timestamp_id, std::time_t)

} // namespace keyword

class logger_base_t {
    bool m_enabled;

protected:
    filter_t m_filter;
    std::unique_ptr<base_frontend_t> m_frontend;
    log::attributes_t m_global_attributes;

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

    void add_attribute(const log::attribute_pair_t& attr) {
        m_global_attributes.insert(attr);
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
                // universe_attributes              // Program global.
                // thread_attributes                // Thread local.
                m_global_attributes,                // Logger object specific.
                std::move(get_scoped_attributes()), // Depending on event scope, e.g. timestamp.
                std::move(local_attributes)         // Any user attributes.
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
        return logger_base_t::open_record({ keyword::severity<Level>() = level }); //!@todo: Неправильно, нужно во-первых уметь генерировать мапу аттрибутов из вариадика, а во-вторых - оставлять operator= для создания одного аттрибута!
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

namespace keyword { DECLARE_KEYWORD(urgent, std::uint8_t) }

TEST(logger_base_t, OpensRecordWhenAttributeFilterSucceeded) {
    logger_base_t log;
    log.set_filter(expr::has_attr(keyword::urgent()));
    log.add_attribute(keyword::urgent() = 1);
    EXPECT_TRUE(log.open_record().valid());
}

TEST(logger_base_t, DoNotOpenRecordWhenAttributeFilterFailed) {
    logger_base_t log;
    log.set_filter(expr::has_attr(keyword::urgent()));
    EXPECT_FALSE(log.open_record().valid());
}


//TEST(logger_base_t, ComplexFilter) {
//    logger_base_t log;
//    log.set_filter(expr::has_attr(keyword::urgent()));// && keyword::urgent() == 1);
//    log.set_attribute(keyword::urgent() = 1);
//    EXPECT_FALSE(log.open_record());
//}

// Allow to make custom filters. severity() >= warning || has_tag(urgent()) && urgent() == 1
//                                         bool(attr)  ||      bool(attr)   && bool(attr)

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
