#pragma once

#include "blackhole/frontend.hpp"
#include "blackhole/sink/syslog.hpp"

namespace blackhole {

template<class Formatter, typename Level>
class frontend_t<Formatter, sink::syslog_t<Level>, Level> : public base_frontend_t {
    const std::unique_ptr<Formatter> m_formatter;
    const std::unique_ptr<sink::syslog_t<Level>> m_sink;
public:
    frontend_t(std::unique_ptr<Formatter> formatter, std::unique_ptr<sink::syslog_t<Level>> sink) :
        m_formatter(std::move(formatter)),
        m_sink(std::move(sink))
    {}

    void handle(const log::record_t& record) {
        auto msg = std::move(m_formatter->format(record));
        auto it = record.attributes.find("severity");

        if (it != record.attributes.end()) {
            const Level level = static_cast<Level>(boost::get<typename std::underlying_type<Level>::type>((*it).second.value));
            m_sink->consume(level, msg);
        }
    }
};

} // namespace blackhole
