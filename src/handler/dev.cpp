#include "blackhole/handler/dev.hpp"

#include <iostream>

#include <boost/variant/variant.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/attributes.hpp"
#include "blackhole/extensions/writer.hpp"
#include "blackhole/handler.hpp"
#include "blackhole/record.hpp"
#include "blackhole/termcolor.hpp"

#include "blackhole/detail/attribute.hpp"
#include "blackhole/detail/datetime.hpp"
#include "blackhole/detail/memory.hpp"
#include "blackhole/detail/util/deleter.hpp"

namespace blackhole {
inline namespace v1 {
namespace handler {

namespace datetime = detail::datetime;

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

class view_visitor : public boost::static_visitor<> {
    std::ostream& stream;

public:
    view_visitor(std::ostream& stream) noexcept :
        stream(stream)
    {}

    auto operator()(std::nullptr_t) const -> void {
        stream << "none";
    }

    template<typename T>
    auto operator()(T value) const -> void {
        stream << value;
    }

    auto operator()(const attribute::view_t::function_type& value) const -> void {
        writer_t wr;
        value(wr);
        stream << wr.result();
    }
};

}  // namespace

class dev_t : public handler_t {
    datetime::generator_t timestamp;

    std::vector<std::function<void(std::ostream& stream, const record_t& record)>> tokens;

public:
    dev_t() :
        timestamp(detail::datetime::make_generator("%Y-%m-%d %H:%M:%S.%f"))
    {
        tokens.emplace_back([](std::ostream& stream, const record_t&) {
            colorize(stream, termcolor_t::gray());
        });

        tokens.emplace_back([&](std::ostream& stream, const record_t& record) {
            const auto timestamp = record.timestamp();
            const auto time = record_t::clock_type::to_time_t(timestamp);
            const auto usec = std::chrono::duration_cast<
                std::chrono::microseconds
            >(timestamp.time_since_epoch()).count() % 1000000;

            std::tm tm;
            ::gmtime_r(&time, &tm);

            fmt::MemoryWriter buffer;
            this->timestamp(buffer, tm, static_cast<std::uint64_t>(usec));
            stream.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        });

        tokens.emplace_back([](std::ostream& stream, const record_t&) {
            stream << " ";
        });

        tokens.emplace_back([](std::ostream& stream, const record_t& record) {
            switch (record.severity()) {
            case 0:
                colorize(stream, termcolor_t::gray());
                stream << "D";
                break;
            case 1:
                colorize(stream, termcolor_t::green());
                stream << "I";
                break;
            case 2:
                colorize(stream, termcolor_t::yellow());
                stream << "W";
                break;
            case 3:
                colorize(stream, termcolor_t::red());
                stream << "E";
                break;
            default:
                colorize(stream, termcolor_t::gray());
                stream << "T";
                break;
            }
        });

        tokens.emplace_back([](std::ostream& stream, const record_t& rec) {
            stream << " " << rec.tid() << "/" << ::getpid();
        });

        tokens.emplace_back([](std::ostream& stream, const record_t&) {
            colorize(stream, termcolor_t::reset());
            colorize(stream, termcolor_t::gray());
        });

        tokens.emplace_back([](std::ostream& stream, const record_t&) {
            stream << " - ";
        });

        tokens.emplace_back([](std::ostream& stream, const record_t&) {
            colorize(stream, termcolor_t::reset());
        });

        tokens.emplace_back([](std::ostream& stream, const record_t& rec) {
            stream << rec.formatted();
        });

        tokens.emplace_back([](std::ostream& stream, const record_t&) {
            colorize(stream, termcolor_t::reset());
        });

        tokens.emplace_back([](std::ostream& stream, const record_t& rec) {
            if (rec.attributes().empty()) {
                return;
            }

            for (const auto& metalink : rec.attributes()) {
                for (const auto& meta : metalink.get()) {
                    stream << std::endl;
                    colorize(stream, termcolor_t::gray());
                    stream << fmt::format("    {:<16.16}: ", meta.first);
                    colorize(stream, termcolor_t::reset());
                    boost::apply_visitor(view_visitor(stream), meta.second.inner().value);
                }
            }
        });
    }

    auto handle(const record_t& record) -> void {
        auto& stream = std::cout;

        std::lock_guard<std::mutex> lock(mutex);
        for (const auto& token : tokens) {
            token(stream, record);
        }

        stream << std::endl;
    }

private:
    static auto colorize(std::ostream& stream, termcolor_t color) -> void {
        if (isatty(stream)) {
            stream << color;
        }
    }
};

}  // namespace handler

using handler::dev_t;

class builder<dev_t>::inner_t {};

builder<dev_t>::builder() :
    d(new inner_t)
{}

auto builder<dev_t>::build() && -> std::unique_ptr<handler_t> {
    return blackhole::make_unique<dev_t>();
}

auto factory<dev_t>::type() const noexcept -> const char* {
    return "dev";
}

auto factory<dev_t>::from(const config::node_t&) const -> std::unique_ptr<handler_t> {
    (void)registry;
    return builder<dev_t>().build();
}

template auto deleter_t::operator()(builder<handler::dev_t>::inner_t*) -> void;

}  // namespace v1
}  // namespace blackhole
