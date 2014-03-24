#pragma once

#include "blackhole/frontend.hpp"
#include "blackhole/sink/files.hpp"

namespace blackhole {

template<class Formatter, class Backend, class Rotator, class Level>
class frontend_t<Formatter, sink::files_t<Backend, Rotator>, Level> : public abstract_frontend_t<Formatter, sink::files_t<Backend, Rotator>> {
public:
    frontend_t(std::unique_ptr<Formatter> formatter, std::unique_ptr<sink::files_t<Backend, Rotator>> sink) :
        abstract_frontend_t<Formatter, sink::files_t<Backend, Rotator>>(std::move(formatter), std::move(sink))
    {}

    void handle(const log::record_t& record) {
        std::string msg = std::move(this->m_formatter->format(record));
        this->m_sink->consume(msg, record.attributes);
    }
};

} // namespace blackhole
