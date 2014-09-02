#pragma once

#include <ostream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/variant.hpp>

#include "blackhole/detail/stream/stream.hpp"
#include "blackhole/formatter/map/value.hpp"
#include "blackhole/formatter/string/parser.hpp"
#include "blackhole/record.hpp"

namespace blackhole {

namespace formatter {

namespace string {

class variadic_visitor_t : public boost::static_visitor<> {
    std::ostringstream& stream;

public:
    variadic_visitor_t(std::ostringstream& stream) :
        stream(stream)
    {}

    template<typename T>
    void operator()(const T& value) const {
        stream << value;
    }

    void operator()(const std::string& value) const {
        stream << "'" << value << "'";
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

    //!@todo: Test prefix/suffix/separator/pattern.
    void operator()(const placeholder::variadic_t& ph) {
        std::vector<std::string> passed;
        passed.reserve(view.upper_size());

        for (auto it = view.begin(); it != view.end(); ++it) {
            const std::string& name = it->first;
            const attribute_t& attribute = it->second;

            //!@todo: This code needs some optimization love.
            std::ostringstream stream;
            stream << "'" << name << "': ";
            variadic_visitor_t visitor(stream);
            boost::apply_visitor(visitor, attribute.value);
            passed.push_back(stream.str());
        }

        stream.rdbuf()->storage()->append(ph.prefix);
        stream.rdbuf()->storage()->append(boost::algorithm::join(passed, ", "));
    }
};

} // namespace string

} // namespace formatter

} // namespace blackhole
