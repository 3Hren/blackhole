#pragma once

#include <memory>
#include <mutex>

#include "blackhole/sink/thread.hpp"
#include "blackhole/record.hpp"

namespace blackhole {

namespace handler {

template<class Formatter>
class formatter_t {
public:
    typedef Formatter formatter_type;

private:
    const std::unique_ptr<formatter_type> formatter;

public:
    formatter_t(std::unique_ptr<formatter_type> formatter) :
        formatter(std::move(formatter))
    {}

    template<typename... Args>
    std::string format(Args&&... args) {
        return formatter->format(std::forward<Args>(args)...);
    }
};

template<class Sink, class = void>
class sink_t;

template<class Sink>
class sink_t<
    Sink,
    typename std::enable_if<
        sink::thread_safety<Sink>::type::value == sink::thread::safety_t::safe
    >::type
> {
public:
    typedef Sink sink_type;

private:
    const std::unique_ptr<sink_type> sink;

public:
    sink_t(std::unique_ptr<sink_type> sink) :
        sink(std::move(sink))
    {}

    template<typename... Args>
    void consume(Args&&... args) {
        sink->consume(std::forward<Args>(args)...);
    }
};

template<class Sink>
class sink_t<
    Sink,
    typename std::enable_if<
        sink::thread_safety<Sink>::type::value == sink::thread::safety_t::unsafe
    >::type
> {
public:
    typedef Sink sink_type;

private:
    const std::unique_ptr<sink_type> sink;
    mutable std::mutex mutex;

public:
    sink_t(std::unique_ptr<sink_type> sink) :
        sink(std::move(sink))
    {}

    template<typename... Args>
    void consume(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex);
        sink->consume(std::forward<Args>(args)...);
    }
};

} // namespace handler

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

    handler::formatter_t<formatter_type> formatter;
    handler::sink_t<sink_type> sink;

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
        this->sink.consume(this->formatter.format(record));
    }
};

} // namespace blackhole
