#include "blackhole/formatter/tskv.hpp"

#include "blackhole/attribute.hpp"
#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/extensions/format.hpp"
#include "blackhole/extensions/writer.hpp"
#include "blackhole/formatter.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/attribute.hpp"
#include "blackhole/detail/datetime.hpp"
#include "blackhole/detail/formatter/string/parser.hpp"
#include "blackhole/detail/formatter/string/token.hpp"
#include "blackhole/detail/memory.hpp"
#include "blackhole/detail/procname.hpp"
#include "blackhole/detail/util/deleter.hpp"

namespace blackhole {
inline namespace v1 {
namespace formatter {

using detail::formatter::string::placeholder::timestamp;
using detail::formatter::string::user;

class tskv_t : public formatter_t {
    timestamp<user> timestamp_token;
    std::map<std::string, std::string> attributes;

public:
    tskv_t() :
        tskv_t(std::map<std::string, std::string>{})
    {}

    tskv_t(std::map<std::string, std::string> attributes) :
        timestamp_token("%Y-%m-%d %H:%M:%S %z", "{}", true),
        attributes(std::move(attributes))
    {}

    auto format(const record_t& record, writer_t& writer) -> void override {
        writer.write("tskv");

        // Format timestamp.
        writer.write("\ttimestamp=");
        const auto timestamp = record.timestamp();
        const auto time = record_t::clock_type::to_time_t(timestamp);
        const auto usec = std::chrono::duration_cast<
            std::chrono::microseconds
        >(timestamp.time_since_epoch()).count() % 1000000;

        std::tm tm;
        if (timestamp_token.gmtime) {
            ::gmtime_r(&time, &tm);
        } else {
            ::localtime_r(&time, &tm);
        }

        fmt::MemoryWriter buffer;
        timestamp_token.generator(buffer, tm, static_cast<std::uint64_t>(usec));
        writer.write(timestamp_token.spec, fmt::StringRef(buffer.data(), buffer.size()));

        writer.write("\tseverity={}", record.severity());
        writer.write("\tpid={}", record.pid());
        writer.write("\ttid={:#x}",
#ifdef __linux__
            record.tid()
#elif __APPLE__
            reinterpret_cast<unsigned long>(record.tid())
#endif
        );
        writer.write("\tmessage={}", fmt::StringRef(record.message().data(), record.message().size()));

        for (const auto& kv : attributes) {
            writer.write("\t{}={}", kv.first, kv.second);
        }

        writer.write("\n");
    }
};

}  // namespace formatter

using formatter::tskv_t;

class builder<tskv_t>::inner_t {
public:
    std::map<std::string, std::string> attributes;
};

builder<tskv_t>::builder() :
    p(new inner_t{}, deleter_t())
{}

auto builder<tskv_t>::insert(const std::string& name, const std::string& value) && -> builder&& {
    p->attributes[name] = value;
    return std::move(*this);
}

auto builder<tskv_t>::build() && -> std::unique_ptr<formatter_t> {
    return blackhole::make_unique<tskv_t>(std::move(p->attributes));
}

auto factory<tskv_t>::type() const noexcept -> const char* {
    return "tskv";
}

auto factory<tskv_t>::from(const config::node_t& config) const -> std::unique_ptr<formatter_t> {
    return blackhole::make_unique<tskv_t>();
}

template auto deleter_t::operator()(builder<formatter::tskv_t>::inner_t* value) -> void;

}  // namespace v1
}  // namespace blackhole
