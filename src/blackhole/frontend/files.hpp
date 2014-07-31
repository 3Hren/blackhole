#pragma once

#include "blackhole/frontend.hpp"
#include "blackhole/sink/files.hpp"

namespace blackhole {

template<class Formatter, class Backend, class Rotator>
class frontend_t<Formatter, sink::files_t<Backend, Rotator>> :
    public abstract_frontend_t<Formatter, sink::files_t<Backend, Rotator>> {

    typedef abstract_frontend_t<
        Formatter, sink::files_t<Backend, Rotator>
    > base_type;
    typedef typename base_type::formatter_type formatter_type;
    typedef typename base_type::sink_type sink_type;

public:
    frontend_t(std::unique_ptr<formatter_type> formatter,
               std::unique_ptr<sink_type> sink) :
        base_type(std::move(formatter), std::move(sink))
    {}

    void handle(const log::record_t& record) {
        this->sink.consume(this->formatter.format(record), record.attributes);
    }
};

} // namespace blackhole
