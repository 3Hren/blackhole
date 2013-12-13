#pragma once

#include "blackhole/frontend.hpp"
#include "blackhole/sink/syslog.hpp"
#include "blackhole/keyword/severity.hpp"

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
        const Level level = record.extract<Level>(keyword::severity<Level>().name());
        m_sink->consume(level, msg);
    }
};

} // namespace blackhole
