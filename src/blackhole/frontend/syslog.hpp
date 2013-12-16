#pragma once

#include "blackhole/frontend.hpp"
#include "blackhole/sink/syslog.hpp"
#include "blackhole/keyword/severity.hpp"

namespace blackhole {

template<class Formatter, typename Level>
class frontend_t<Formatter, sink::syslog_t<Level>, Level> : public abstract_frontend_t<Formatter, sink::syslog_t<Level>> {
public:
    frontend_t(std::unique_ptr<Formatter> formatter, std::unique_ptr<sink::syslog_t<Level>> sink) :
        abstract_frontend_t<Formatter, sink::syslog_t<Level>>(std::move(formatter), std::move(sink))
    {}

    void handle(const log::record_t& record) {
        const Level level = record.extract<Level>(keyword::severity<Level>().name());
        std::string msg = std::move(this->m_formatter->format(record));
        this->m_sink->consume(level, msg);
    }
};

} // namespace blackhole
