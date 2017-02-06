#include "blackhole/formatter/tskv.hpp"

#include <set>

#include <boost/optional/optional.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/extensions/format.hpp"
#include "blackhole/extensions/writer.hpp"
#include "blackhole/formatter.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/attribute.hpp"
#include "blackhole/detail/datetime.hpp"
#include "blackhole/detail/formatter/string/token.hpp"
#include "blackhole/detail/memory.hpp"
#include "blackhole/detail/util/deleter.hpp"

namespace blackhole {
inline namespace v1 {
namespace formatter {

namespace {

class view_visitor : public boost::static_visitor<> {
    writer_t& writer;
    const std::string& spec;

public:
    view_visitor(writer_t& writer, const std::string& spec) noexcept :
        writer(writer),
        spec(spec)
    {}

    auto operator()(std::nullptr_t) const -> void {
        writer.write(spec, "none");
    }

    template<typename T>
    auto operator()(T value) const -> void {
        writer.write(spec, value);
    }

    auto operator()(const string_view& value) const -> void {
        writer.write(spec, value.data());
    }

    auto operator()(const attribute::view_t::function_type& value) const -> void {
        value(writer);
    }
};

}  // namespace

using detail::formatter::string::placeholder::timestamp;
using detail::formatter::string::user;

struct tskv_data {
    std::set<std::string> removed;
    std::map<std::string, std::string> renamed;
    std::map<std::string, std::string> attributes;
    std::map<std::string, timestamp<user>> timestamps;
};

class builder_t {
    writer_t& wr;
    const record_t& record;
    const tskv_data& data;

public:
    builder_t(writer_t& wr, const record_t& record, const tskv_data& data) :
        wr(wr),
        record(record),
        data(data)
    {}

    auto add_header() -> void {
        wr.write("tskv");
    }

    auto add_severity() -> void {
        add("severity", record.severity());
    }

    auto add_timestamps() -> void {
        for (const auto& kv : data.timestamps) {
            write_timestamp(record.timestamp(), kv.first, kv.second);
        }
    }

    auto add_pid() -> void {
        add("pid", record.pid());
    }

    auto add_tid() -> void {
        wr.write("\ttid={:#x}",
#ifdef __linux__
            record.tid()
#elif __APPLE__
            reinterpret_cast<unsigned long>(record.tid())
#endif
        );
    }

    auto add_message() -> void {
        add("message", record.formatted());
    }

    auto add_attributes() -> void {
        for (const auto& attributes : record.attributes()) {
            for (const auto& attribute : attributes.get()) {
                writer_t wr;
                boost::apply_visitor(view_visitor(wr, "{}"), attribute.second.inner().value);
                add(attribute.first.to_string(), string_view(wr.inner.data(), wr.inner.size()));
            }
        }
    }

    auto add_constants() -> void {
        for (const auto& kv : data.attributes) {
            add(kv.first, kv.second);
        }
    }

    auto finish() -> void {
        wr.write("\n");
    }

private:
    auto add(const std::string& name, string_view value) -> void {
        if (data.removed.count(name) > 0) {
            return;
        }
        add_key(name);
        wr.write("=");
        write_escaped_val(value);
    }

    template<typename T>
    auto add(const std::string& name, const T& value) -> void {
        if (data.removed.count(name) > 0) {
            return;
        }
        add_key(name);
        wr.write("={}", value);
    }

    auto add_key(const std::string& name) -> void {
        string_view renamed;
        const auto it = data.renamed.find(name);
        if (it == data.renamed.end()) {
            renamed = name;
        } else {
            renamed = it->second;
        }

        wr.write("\t");
        write_escaped_key(renamed);
    }

    auto write_timestamp(record_t::time_point value, const std::string& name, timestamp<user> token) -> void {
        const auto time = record_t::clock_type::to_time_t(value);
        const auto usec = std::chrono::duration_cast<
            std::chrono::microseconds
        >(value.time_since_epoch()).count() % 1000000;

        std::tm tm;
        if (token.gmtime) {
            ::gmtime_r(&time, &tm);
        } else {
            ::localtime_r(&time, &tm);
        }

        fmt::MemoryWriter buffer;
        token.generator(buffer, tm, static_cast<std::uint64_t>(usec));

        add(name, string_view(buffer.data(), buffer.size()));
    }

    auto write_escaped_key(string_view key) -> void {
        for (auto it = key.data(); it != key.data() + key.size(); ++it) {
            switch (*it) {
            case '=':
                wr.inner << "\\=";
                break;
            default:
                wr.inner << *it;
            };
        }
    }

    auto write_escaped_val(string_view value) -> void {
        for (auto it = value.data(); it != value.data() + value.size(); ++it) {
            switch (*it) {
            case '\a':
                wr.inner << "\\a";
                break;
            case '\b':
                wr.inner << "\\b";
                break;
            case '\t':
                wr.inner << "\\t";
                break;
            case '\n':
                wr.inner << "\\n";
                break;
            case '\v':
                wr.inner << "\\v";
                break;
            case '\f':
                wr.inner << "\\f";
                break;
            case '\r':
                wr.inner << "\\r";
                break;
            case '\x1B':
                wr.inner << "\\e";
                break;
            case '\0':
                wr.inner << "\\0";
                break;
            case '\\':
                wr.inner << "\\\\";
                break;
            default:
                wr.inner << *it;
            };
        }
    }
};

class tskv_t : public formatter_t {
    tskv_data d;

public:
    explicit tskv_t() {}
    explicit tskv_t(tskv_data data) :
        d(std::move(data))
    {}

    auto format(const record_t& record, writer_t& writer) -> void override {
        builder_t builder{writer, record, d};
        builder.add_header();
        builder.add_constants();
        builder.add_timestamps();
        builder.add_severity();
        builder.add_pid();
        builder.add_tid();
        builder.add_message();

        builder.add_attributes();

        builder.finish();
    }

private:
    auto write_escaped_key(writer_t& writer, string_view data) -> void {
        for (auto it = data.data(); it != data.data() + data.size(); ++it) {
            switch (*it) {
            case '=':
                writer.inner << "\\=";
                break;
            default:
                writer.inner << *it;
            };
        }
    }

    auto write_escaped_val(writer_t& writer, string_view data) -> void {
        for (auto it = data.data(); it != data.data() + data.size(); ++it) {
            switch (*it) {
            case '\a':
                writer.inner << "\\a";
                break;
            case '\b':
                writer.inner << "\\b";
                break;
            case '\t':
                writer.inner << "\\t";
                break;
            case '\n':
                writer.inner << "\\n";
                break;
            case '\v':
                writer.inner << "\\v";
                break;
            case '\f':
                writer.inner << "\\f";
                break;
            case '\r':
                writer.inner << "\\r";
                break;
            case '\x1B':
                writer.inner << "\\e";
                break;
            case '\0':
                writer.inner << "\\0";
                break;
            case '\\':
                writer.inner << "\\\\";
                break;
            default:
                writer.inner << *it;
            };
        }
    }
};

}  // namespace formatter

using formatter::tskv_t;
using formatter::tskv_data;

class builder<tskv_t>::inner_t {
public:
    tskv_data data;
};

builder<tskv_t>::builder() :
    p(new inner_t{}, deleter_t())
{
    p->data.timestamps["timestamp"] = {"%Y-%m-%d %H:%M:%S %z", "{}", true};
}

auto builder<tskv_t>::create(const std::string& name, const std::string& value) & -> builder& {
    p->data.attributes[name] = value;
    return *this;
}

auto builder<tskv_t>::create(const std::string& name, const std::string& value) && -> builder&& {
    p->data.attributes[name] = value;
    return std::move(*this);
}

auto builder<tskv_t>::rename(const std::string& from, const std::string& to) & -> builder& {
    p->data.renamed[from] = to;
    return *this;
}

auto builder<tskv_t>::rename(const std::string& from, const std::string& to) && -> builder&& {
    p->data.renamed[from] = to;
    return std::move(*this);
}

auto builder<tskv_t>::remove(const std::string& name) & -> builder& {
    p->data.removed.insert(name);
    return *this;
}

auto builder<tskv_t>::remove(const std::string& name) && -> builder&& {
    p->data.removed.insert(name);
    return std::move(*this);
}

auto builder<tskv_t>::timestamp(const std::string& name, const std::string& value) & -> builder& {
    p->data.timestamps[name] = formatter::timestamp<formatter::user>(value, "{}", true);
    return *this;
}

auto builder<tskv_t>::timestamp(const std::string& name, const std::string& value) && -> builder&& {
    p->data.timestamps[name] = formatter::timestamp<formatter::user>(value, "{}", true);
    return std::move(*this);
}

auto builder<tskv_t>::build() && -> std::unique_ptr<formatter_t> {
    return blackhole::make_unique<tskv_t>(std::move(p->data));
}

auto factory<tskv_t>::type() const noexcept -> const char* {
    return "tskv";
}

auto factory<tskv_t>::from(const config::node_t& config) const -> std::unique_ptr<formatter_t> {
    builder<tskv_t> builder;

    if (auto cfg = config["create"]) {
        cfg.each_map([&](const std::string& key, const config::node_t& value) {
            builder.create(key, value.to_string());
        });
    }

    if (auto cfg = config["rename"]) {
        cfg.each_map([&](const std::string& from, const config::node_t& to) {
            builder.rename(from, to.to_string());
        });
    }

    if (auto cfg = config["remove"]) {
        cfg.each([&](const config::node_t& key) {
            builder.remove(key.to_string());
        });
    }

    if (auto cfg = config["mutate"]) {
        cfg.each_map([&](const std::string& key, const config::node_t& how) {
            if (auto strftime = how["strftime"]) {
                builder.timestamp(key, strftime.to_string().get());
                return;
            }
        });
    }

    return std::move(builder).build();
}

template auto deleter_t::operator()(builder<formatter::tskv_t>::inner_t* value) -> void;

}  // namespace v1
}  // namespace blackhole
