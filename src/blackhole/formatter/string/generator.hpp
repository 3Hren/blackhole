#pragma once

#include <ostream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/variant.hpp>

#include "blackhole/detail/stream/stream.hpp"
#include "blackhole/error.hpp"
#include "blackhole/formatter/map/value.hpp"
#include "blackhole/formatter/string/parser.hpp"
#include "blackhole/record.hpp"

namespace blackhole {

namespace formatter {

namespace string {

class variadic_visitor_t : public boost::static_visitor<> {
    const attribute::name_t& name;
    const attribute::value_t& value;
    std::ostringstream& stream;

public:
    variadic_visitor_t(const attribute::name_t& name,
                       const attribute::value_t& value,
                       std::ostringstream& stream) :
        name(name),
        value(value),
        stream(stream)
    {}

    void operator()(const literal_t& literal) {
        stream << literal.value;
    }

    void operator()(const placeholder::variadic_t::key_t&) {
        stream << name;
    }

    void operator()(const placeholder::variadic_t::value_t) {
        stream << value;
    }
};

class visitor_t : public boost::static_visitor<> {
    stickystream_t& stream;
    const mapping::value_t& mapper;
    const attribute::set_view_t& view;

public:
    visitor_t(stickystream_t& stream,
              const mapping::value_t& mapper,
              const attribute::set_view_t& view) :
        stream(stream),
        mapper(mapper),
        view(view)
    {}

    void operator()(const literal_t& literal) {
        stream.rdbuf()->storage()->append(literal.value);
    }

    void operator()(const placeholder::required_t& ph) {
        if (auto attribute = view.find(ph.name)) {
            mapper(stream, ph.name, attribute->value);
            return;
        }

        throw error_t("required attribute '%s' was not provided", ph.name);
    }

    void operator()(const placeholder::optional_t& ph) {
        if (auto attribute = view.find(ph.name)) {
            stream.rdbuf()->storage()->append(ph.prefix);
            mapper(stream, ph.name, attribute->value);
            stream.rdbuf()->storage()->append(ph.suffix);
        }
    }

    //!@todo: 1. Test pattern.
    void operator()(const placeholder::variadic_t& ph) {
        std::vector<std::string> passed;
        passed.reserve(view.upper_size());

        //!@todo: 3. Call begin() & end() with parameter.
        for (auto it = view.begin(); it != view.end(); ++it) {
            //!@todo: 2. This code needs some optimization love.
            std::ostringstream stream;
            variadic_visitor_t visitor(it->first, it->second.value, stream);
            for (auto t = ph.pattern.begin(); t != ph.pattern.end(); ++t) {
                boost::apply_visitor(visitor, *t);
            }

            passed.push_back(stream.str());
        }

        if (!passed.empty()) {
            stream.rdbuf()->storage()->append(ph.prefix);
            stream.rdbuf()->storage()->append(
                boost::algorithm::join(passed, ph.separator)
            );
            stream.rdbuf()->storage()->append(ph.suffix);
        }
    }
};

} // namespace string

} // namespace formatter

} // namespace blackhole
