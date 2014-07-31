#pragma once

#include "blackhole/frontend.hpp"
#include "blackhole/sink/syslog.hpp"
#include "blackhole/keyword/severity.hpp"

namespace blackhole {

template<class Formatter, typename Level>
class frontend_t<Formatter, sink::syslog_t<Level>> :
    public abstract_frontend_t<Formatter, sink::syslog_t<Level>> {

    typedef abstract_frontend_t<Formatter, sink::syslog_t<Level>> base_type;
    typedef typename base_type::formatter_type formatter_type;
    typedef typename base_type::sink_type sink_type;

public:
    frontend_t(std::unique_ptr<formatter_type> formatter,
               std::unique_ptr<sink_type> sink) :
        base_type(std::move(formatter), std::move(sink))
    {}

    void handle(const log::record_t& record) {
        const Level level = record.extract<Level>(keyword::severity<Level>().name());
        this->sink.consume(level, this->formatter.format(record));
    }
};

} // namespace blackhole
