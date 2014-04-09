#pragma once

#include <memory>

#include "record.hpp"

namespace blackhole {

class base_frontend_t {
public:
    virtual ~base_frontend_t() {}
    virtual void handle(const log::record_t& record) = 0;
};

template<class Formatter, class Sink>
class abstract_frontend_t : public base_frontend_t {
protected:
    const std::unique_ptr<Formatter> m_formatter;
    const std::unique_ptr<Sink> m_sink;

public:
    abstract_frontend_t(std::unique_ptr<Formatter> formatter, std::unique_ptr<Sink> sink) :
        m_formatter(std::move(formatter)),
        m_sink(std::move(sink))
    {}
};

template<class Formatter, class Sink>
class frontend_t : public abstract_frontend_t<Formatter, Sink> {
public:
    frontend_t(std::unique_ptr<Formatter> formatter, std::unique_ptr<Sink> sink) :
        abstract_frontend_t<Formatter, Sink>(std::move(formatter), std::move(sink))
    {}

    void handle(const log::record_t& record) {
        std::string msg = std::move(this->m_formatter->format(record));
        this->m_sink->consume(msg);
    }
};

} // namespace blackhole
