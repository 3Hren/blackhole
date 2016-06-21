#include "blackhole/sink/console.hpp"

#include <unistd.h>

#include <iostream>
#include <map>
#include <mutex>

#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/record.hpp"
#include "blackhole/registry.hpp"
#include "blackhole/severity.hpp"
#include "blackhole/stdext/string_view.hpp"
#include "blackhole/termcolor.hpp"

#include "blackhole/detail/memory.hpp"
#include "blackhole/detail/util/deleter.hpp"

#include "console.hpp"
#include "../filter/zen.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"

// Both standard output and error access mutex. Messages written with Blackhole will be
// synchronized, otherwise an intermixing can occur.
static std::mutex mutex;

#pragma clang diagnostic pop

namespace {

auto streamfd(const std::ostream& stream) noexcept -> FILE* {
    if (&stream == &std::cout) {
        return stdout;
    } else if (&stream == &std::cerr) {
        return stderr;
    } else {
        return nullptr;
    }
}

auto isatty(const std::ostream& stream) -> bool {
    if (auto file = streamfd(stream)) {
#if defined(__linux__) || defined(__APPLE__)
        return ::isatty(::fileno(file));
#else
#error unsupported platform
#endif
    } else {
        return false;
    }
}

}  // namespace

console_t::console_t() :
    stream_(std::cout),
    filter(new filter::zen_t),
    mapping_([](const record_t&) -> termcolor_t { return {}; })
{}

console_t::console_t(std::unique_ptr<filter_t> filter) :
    stream_(std::cout),
    filter(std::move(filter)),
    mapping_([](const record_t&) -> termcolor_t { return {}; })
{}

console_t::console_t(std::ostream& stream, std::function<termcolor_t(const record_t& record)> mapping) :
    stream_(stream),
    filter(new filter::zen_t),
    mapping_(std::move(mapping))
{}

auto console_t::stream() noexcept -> std::ostream& {
    return stream_;
}

auto console_t::mapping(const record_t& record) const -> termcolor_t {
    return mapping_(record);
}

auto console_t::emit(const record_t& record, const string_view& formatted) -> void {
    switch (filter->filter(record)) {
    case filter_t::action_t::neutral:
    case filter_t::action_t::accept:
        break;
    case filter_t::action_t::deny:
        return;
    }

    if (isatty(stream())) {
        std::lock_guard<std::mutex> lock(mutex);
        mapping(record)
            .write(stream(), formatted.data(), formatted.size());
        stream() << std::endl;
    } else {
        std::lock_guard<std::mutex> lock(mutex);
        stream().write(formatted.data(), static_cast<std::streamsize>(formatted.size()));
        stream() << std::endl;
    }
}

}  // namespace sink

class builder<sink::console_t>::inner_t {
public:
    std::ostream* stream;
    std::function<termcolor_t(const record_t& record)> mapping;
};

builder<sink::console_t>::builder() :
    d(new inner_t{&std::cout, [](const record_t&) -> termcolor_t { return {}; }})
{}

auto builder<sink::console_t>::stdout() & -> builder& {
    d->stream = &std::cout;
    return *this;
}

auto builder<sink::console_t>::stdout() && -> builder&& {
    return std::move(stdout());
}

auto builder<sink::console_t>::stderr() & -> builder& {
    d->stream = &std::cerr;
    return *this;
}

auto builder<sink::console_t>::stderr() && -> builder&& {
    return std::move(stderr());
}

auto builder<sink::console_t>::colorize(severity_t severity, termcolor_t color) & -> builder& {
    const auto fallback = std::move(d->mapping);

    return colorize([=](const record_t& record) -> termcolor_t {
        if (severity == record.severity()) {
            return color;
        } else {
            return fallback(record);
        }
    });
}

auto builder<sink::console_t>::colorize(severity_t severity, termcolor_t color) && -> builder&& {
    return std::move(colorize(severity, color));
}

auto builder<sink::console_t>::colorize(std::function<termcolor_t(const record_t& record)> fn) & -> builder& {
    d->mapping = std::move(fn);
    return *this;
}

auto builder<sink::console_t>::colorize(std::function<termcolor_t(const record_t& record)> fn) && -> builder&& {
    return std::move(colorize(std::move(fn)));
}

auto builder<sink::console_t>::build() && -> std::unique_ptr<sink_t> {
    return blackhole::make_unique<sink::console_t>(*d->stream, std::move(d->mapping));
}

auto factory<sink::console_t>::type() const noexcept -> const char* {
    return "console";
}

auto factory<sink::console_t>::from(const config::node_t& config) const -> std::unique_ptr<sink_t> {
    if (auto type = config["filter"]["type"].to_string()) {
        auto factory = registry.filter(*type);
        auto filter = factory(*config["filter"].unwrap());

        return blackhole::make_unique<sink::console_t>(std::move(filter));
    }

    return blackhole::make_unique<sink::console_t>();
}

template auto deleter_t::operator()(builder<sink::console_t>::inner_t* value) -> void;

}  // namespace v1
}  // namespace blackhole
