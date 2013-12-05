#pragma once

#include <memory>

#include "record.hpp"

namespace blackhole {

class base_frontend_t {
public:
    virtual void handle(const log::record_t& record) = 0;
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

    void handle(const log::record_t& record) {
        auto msg = std::move(m_formatter->format(record));
        m_sink->consume(msg);
    }
};

} // namespace blackhole
