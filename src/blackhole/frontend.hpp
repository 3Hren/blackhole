#pragma once

#include <memory>

#include "record.hpp"

namespace blackhole {

//!@todo: Consider setting mapper for entire frontend or maybe logger.
class base_frontend_t {
public:
    virtual ~base_frontend_t() {}
    virtual void handle(const log::record_t& record) = 0;
};

template<class Formatter, class Sink>
class abstract_frontend_t : public base_frontend_t {
protected:
    typedef Formatter formatter_type;
    typedef Sink sink_type;

    const std::unique_ptr<formatter_type> formatter;
    const std::unique_ptr<sink_type> sink;

public:
    abstract_frontend_t(std::unique_ptr<formatter_type> formatter,
                        std::unique_ptr<sink_type> sink) :
        formatter(std::move(formatter)),
        sink(std::move(sink))
    {}
};

template<class Formatter, class Sink>
class frontend_t : public abstract_frontend_t<Formatter, Sink> {
    typedef abstract_frontend_t<Formatter, Sink> base_type;
    typedef typename base_type::formatter_type formatter_type;
    typedef typename base_type::sink_type sink_type;

public:
    frontend_t(std::unique_ptr<formatter_type> formatter,
               std::unique_ptr<sink_type> sink) :
        base_type(std::move(formatter), std::move(sink))
    {}

    void handle(const log::record_t& record) {
        std::string msg = std::move(this->formatter->format(record));
        this->sink->consume(msg);
    }
};

} // namespace blackhole
