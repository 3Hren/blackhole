#pragma once

#include <ostream>
#include <string>
#include <unordered_set>

#include <boost/algorithm/string.hpp>
#include <boost/variant/variant.hpp>

#include "blackhole/record.hpp"

namespace blackhole {
namespace detail {
namespace formatter {
namespace string {

typedef std::function<void(const record_t::time_point&, writer_t&)> timestamp_formatter;
typedef std::function<void(int, writer_t&)> severity_formatter;

// class variadic_visitor_t : public boost::static_visitor<> {
//     const attribute::name_t& name;
//     const attribute::value_t& value;
//     stickystream_t& stream;
//
// public:
//     variadic_visitor_t(const attribute::name_t& name,
//                        const attribute::value_t& value,
//                        stickystream_t& stream) :
//         name(name),
//         value(value),
//         stream(stream)
//     {}
//
//     void operator()(const literal_t& literal) {
//         stream << literal.value;
//     }
//
//     void operator()(const placeholder::variadic_t::key_t&) {
//         stream << name;
//     }
//
//     void operator()(const placeholder::variadic_t::value_t) {
//         stream << value;
//     }
// };

class visitor_t : public boost::static_visitor<> {
    const record_t& record;
    writer_t& writer;
    bool unique;

public:
    timestamp_formatter tfn;
    severity_formatter sfn;

    visitor_t(const record_t& record, writer_t& writer, bool unique) :
        record(record),
        writer(writer),
        unique(unique)
    {}

    void operator()(const literal_t& literal) {
        writer.inner << literal.value;
    }

    void operator()(const placeholder::required_t& ph) {
        if (ph.name == "message") {
            writer.write(record.formatted().data());
            return;
        } else if (ph.name == "severity") {
            sfn(record.severity(), writer);
            return;
        } else if (ph.name == "timestamp") {
            tfn(record.timestamp(), writer);
            return;
        }
            // TODO: find
        // if (auto attribute = view.find(ph.name)) {
        //     mapper(stream, ph.name, attribute->value);
        //     return;
        // }

        throw std::runtime_error("required attribute '%s' was not provided");//, ph.name);
    }

    void operator()(const placeholder::optional_t& ph) {
        // if (auto attribute = view.find(ph.name)) {
        //     stream.rdbuf()->storage()->append(ph.prefix);
        //     mapper(stream, ph.name, attribute->value);
        //     stream.rdbuf()->storage()->append(ph.suffix);
        // }
    }

    void operator()(const placeholder::variadic_t& ph) {
        // const auto& attributes = this->view.partial();
        //
        // if (attributes.empty()) {
        //     return;
        // }
        //
        // stream.rdbuf()->storage()->append(ph.prefix);
        //
        // if (filter) {
        //     std::unordered_set<std::string> set;
        //     auto it = attributes.rbegin();
        //     traverse(ph.pattern, it->first, it->second.value, stream);
        //
        //     set.insert(it->first);
        //
        //     it++;
        //
        //     for (; it != attributes.rend(); ++it) {
        //         if (set.count(it->first)) {
        //             continue;
        //         } else {
        //             set.insert(it->first);
        //         }
        //
        //         stream.rdbuf()->storage()->append(ph.separator);
        //         traverse(ph.pattern, it->first, it->second.value, stream);
        //     }
        // } else {
        //     auto it = attributes.begin();
        //     traverse(ph.pattern, it->first, it->second.value, stream);
        //     it++;
        //
        //     for (; it != attributes.end(); ++it) {
        //         stream.rdbuf()->storage()->append(ph.separator);
        //         traverse(ph.pattern, it->first, it->second.value, stream);
        //     }
        // }
        //
        // stream.rdbuf()->storage()->append(ph.suffix);
    }

private:
    // static
    // inline
    // void
    // traverse(const std::vector<placeholder::variadic_t::pattern_t>& pattern,
    //          const attribute::name_t& name,
    //          const attribute::value_t& value,
    //          stickystream_t& stream)
    // {
    //     // variadic_visitor_t visitor(name, value, stream);
    //     // for (auto it = pattern.begin(); it != pattern.end(); ++it) {
    //     //     boost::apply_visitor(visitor, *it);
    //     //     stream.flush();
    //     // }
    // }
};

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace blackhole
