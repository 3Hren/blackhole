#include "blackhole/sink/console.hpp"

#include <unistd.h>

#include <iostream>
#include <mutex>

#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/termcolor.hpp"

#include "blackhole/detail/memory.hpp"
#include "blackhole/detail/util/deleter.hpp"

#include "console.hpp"

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
    colormap([](const record_t&) -> termcolor_t { return {}; })
{}

console_t::console_t(std::ostream& stream, termcolor_map colormap) :
    stream_(stream),
    colormap(std::move(colormap))
{}

auto console_t::stream() noexcept -> std::ostream& {
    return stream_;
}

auto console_t::emit(const record_t& record, const string_view& formatted) -> void {
    if (isatty(stream())) {
        std::lock_guard<std::mutex> lock(mutex);
        colormap(record)
            .write(stream(), formatted.data(), formatted.size());
        stream() << std::endl;
    } else {
        std::lock_guard<std::mutex> lock(mutex);
        stream().write(formatted.data(), static_cast<std::streamsize>(formatted.size()));
        stream() << std::endl;
    }
}

}  // namespace sink

namespace experimental {

class builder<sink::console_t>::inner_t {
public:
    std::ostream* stream;
    sink::termcolor_map termcolor;
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

auto builder<sink::console_t>::build() && -> std::unique_ptr<sink_t> {
    return blackhole::make_unique<sink::console_t>(*d->stream, std::move(d->termcolor));
}

auto factory<sink::console_t>::type() const noexcept -> const char* {
    return "console";
}

auto factory<sink::console_t>::from(const config::node_t&) const -> std::unique_ptr<sink_t> {
    return blackhole::make_unique<sink::console_t>();
}

}  // namespace experimental

template auto deleter_t::operator()(experimental::builder<sink::console_t>::inner_t* value) -> void;

}  // namespace v1
}  // namespace blackhole
