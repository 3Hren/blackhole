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
    stickystream_t& stream;

public:
    variadic_visitor_t(const attribute::name_t& name,
                       const attribute::value_t& value,
                       stickystream_t& stream) :
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

    void operator()(const placeholder::variadic_t& ph) {
        typedef attribute::set_view_t::attached_set_t attached_type;
        typedef attribute::set_view_t::external_set_t external_type;
        auto view = attribute::partial_view_t<
            external_type,
            attached_type
        >(this->view);

        if (view.empty()) {
            return;
        }

        stream.rdbuf()->storage()->append(ph.prefix);
        auto it = view.begin();
        traverse(ph.pattern, it->first, it->second.value, stream);
        it++;

        for (; it != view.end(); ++it) {
            stream.rdbuf()->storage()->append(ph.separator);
            traverse(ph.pattern, it->first, it->second.value, stream);
        }

        stream.rdbuf()->storage()->append(ph.suffix);
    }

private:
    static
    inline
    void
    traverse(const std::vector<placeholder::variadic_t::pattern_t>& pattern,
             const attribute::name_t& name,
             const attribute::value_t& value,
             stickystream_t& stream)
    {
        variadic_visitor_t visitor(name, value, stream);
        for (auto it = pattern.begin(); it != pattern.end(); ++it) {
            boost::apply_visitor(visitor, *it);
            stream.flush();
        }
    }
};

} // namespace string

} // namespace formatter

} // namespace blackhole
