#pragma once

#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/detail/stream/stream.hpp"
#include "blackhole/error.hpp"
#include "blackhole/formatter/map/value.hpp"

namespace blackhole {

namespace formatter {

namespace string {

static const char VARIADIC_KEY_PREFIX[] = "...";
static const std::size_t VARIADIC_KEY_PRFFIX_LENGTH = std::strlen(VARIADIC_KEY_PREFIX);

namespace builder {

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

struct variadic_t {
    typedef log::attribute::scope_underlying_type scope_underlying_type;

    const std::string placeholder;

    void operator ()(blackhole::aux::attachable_ostringstream& stream, const mapping::value_t&, const log::attributes_t& attributes) const {
        std::vector<std::string> passed;
        passed.reserve(attributes.size());

        for (auto pit = placeholder.begin() + string::VARIADIC_KEY_PRFFIX_LENGTH; pit != placeholder.end(); ++pit) {
            const scope_underlying_type scope = *pit - '0';

            for (auto it = attributes.begin(); it != attributes.end(); it++) {
                const std::string& name = it->first;
                const log::attribute_t& attribute = it->second;
                if (static_cast<scope_underlying_type>(attribute.scope) & scope) {
                    std::ostringstream stream;
                    variadic_visitor_t visitor(stream);
                    stream << "'" << name << "': ";
                    boost::apply_visitor(visitor, attribute.value);
                    passed.push_back(stream.str());
                }
            }
        }
        stream.rdbuf()->storage()->append(boost::algorithm::join(passed, ", "));
    }
};

} // namespace builder

} // namespace string

} // namespace formatter

} // namespace blackhole
