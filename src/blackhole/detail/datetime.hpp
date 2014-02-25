#pragma once

#include <ctime>
#include <functional>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/range.hpp>
#include <boost/version.hpp>
#include <boost/utility.hpp>

#include "blackhole/detail/stream/stream.hpp"

namespace blackhole {

namespace aux {

struct context_t {
    attachable_ostringstream& stream;
    std::string& str;
    std::tm tm;
};

inline void fill(std::string& str, int value, uint length, char filler = '0') {
    char buffer[std::numeric_limits<uint32_t>::digits10 + 2];
    uint digits = 0;
    do {
        buffer[digits] = (value % 10) + '0';
        value /= 10;
        digits++;
    } while (value);

    BOOST_ASSERT(length >= digits);
    const uint gap = length - digits;
    for (uint i = 0; i < gap; ++i) {
        str.push_back(filler);
    }

    for (uint i = 0; i < digits; ++i) {
        str.push_back(buffer[digits - 1 - i]);
    }
}

namespace visit {

namespace year {

inline void full(context_t& context) {
    fill(context.str, context.tm.tm_year, 4);
}

inline void normal(context_t& context) {
    fill(context.str, context.tm.tm_year % 100, 2);
}

} // namespace year

namespace month {

inline void numeric(context_t& context) {
    fill(context.str, context.tm.tm_mon, 2);
}

} // namespace month

namespace day {

namespace month {

template<char Filler = '0'>
inline void numeric(context_t& context) {
    fill(context.str, context.tm.tm_mday, 2, Filler);
}

} // namespace month

} // namespace day

namespace hour {

inline void h24(context_t& context) {
    fill(context.str, context.tm.tm_hour, 2);
}

inline void h12(context_t& context) {
    const uint mod = context.tm.tm_hour % 12;
    fill(context.str, mod == 0 ? 12 : mod, 2);
}

} // namespace hour

namespace time {

namespace minute {

inline void normal(context_t& context) {
    fill(context.str, context.tm.tm_min, 2);
}

} // namespace minute

namespace second {

inline void normal(context_t& context) {
    fill(context.str, context.tm.tm_sec, 2);
}

} // namespace second

} // namespace time

} // namespace visit

struct literal_generator_t {
    std::string literal;

    void operator()(context_t& context) const {
        context.str.append(literal);
    }
};

namespace datetime {

typedef std::function<void(context_t&)> generator_action_t;

//!@note: copyable, movable.
class generator_t {
    std::vector<generator_action_t> actions;
public:
    generator_t(std::vector<generator_action_t>&& actions) :
        actions(std::move(actions))
    {}

    template<class Stream>
    void operator()(Stream& stream, const std::tm& tm) const {
        context_t context { stream, *stream.rdbuf()->storage(), tm };

        for (auto it = actions.begin(); it != actions.end(); ++it) {
            const generator_action_t& action = *it;
            action(context);
        }
    }
};

class generator_handler_t {
    typedef std::string::const_iterator iterator_type;

    std::vector<generator_action_t>& actions;
    std::string m_literal;
public:
    generator_handler_t(std::vector<generator_action_t>& actions) :
        actions(actions)
    {}

    virtual void partial_literal(iterator_type begin, iterator_type end) {
        m_literal.append(begin, end);
    }

    virtual void end_partial_literal() {
        if (!m_literal.empty()) {
            literal(boost::make_iterator_range(m_literal));
            m_literal.clear();
        }
    }

    virtual void literal(const boost::iterator_range<iterator_type>& range) {
        actions.push_back(literal_generator_t { boost::copy_range<std::string>(range) });
    }

    virtual void placeholder(const boost::iterator_range<iterator_type>& range) {
        literal(range);
    }

    virtual void full_year() {
        end_partial_literal();
        actions.push_back(&visit::year::full);
    }

    virtual void short_year() {
        end_partial_literal();
        actions.push_back(&visit::year::normal);
    }

    virtual void numeric_month() {
        end_partial_literal();
        actions.push_back(&visit::month::numeric);
    }

    virtual void month_day(bool has_leading_zero = true) {
        end_partial_literal();
        if (has_leading_zero) {
            actions.push_back(&visit::day::month::numeric<'0'>);
        } else {
            actions.push_back(&visit::day::month::numeric<' '>);
        }
    }

    virtual void hours() {
        end_partial_literal();
        actions.push_back(&visit::hour::h24);
    }

    virtual void hours12() {
        end_partial_literal();
        actions.push_back(&visit::hour::h12);
    }

    virtual void minute() {
        end_partial_literal();
        actions.push_back(&visit::time::minute::normal);
    }

    virtual void second() {
        end_partial_literal();
        actions.push_back(&visit::time::second::normal);
    }
};

namespace parser {

class through_t {
public:
    typedef std::string::const_iterator iterator_type;

    static iterator_type parse(iterator_type it, iterator_type end, generator_handler_t& handler) {
        BOOST_ASSERT(it != end && it + 1 != end);
        handler.placeholder(boost::make_iterator_range(it, it + 2));
        return it + 2;
    }
};

template<class Decorate = through_t>
class common {
public:
    typedef typename Decorate::iterator_type iterator_type;
};

template<class Decorate>
class time {
public:
    typedef typename Decorate::iterator_type iterator_type;

    static iterator_type parse(iterator_type it, iterator_type end, generator_handler_t& handler) {
        BOOST_ASSERT(it != end && it + 1 != end);
        switch (*(it + 1)) {
        case 'H':
            handler.hours();
            break;
        case 'I':
            handler.hours12();
            break;
        case 'M':
            handler.minute();
            break;
        case 'S':
            handler.second();
            break;
        default:
            return Decorate::parse(it, end, handler);
        }

        return it + 2;
    }
};

template<class Decorate>
class date {
public:
    typedef typename Decorate::iterator_type iterator_type;

    static iterator_type parse(iterator_type it, iterator_type end, generator_handler_t& handler) {
        BOOST_ASSERT(it != end && it + 1 != end);
        switch (*(it + 1)) {
        case 'Y':
            handler.full_year();
            break;
        case 'y':
            handler.short_year();
            break;
        case 'm':
            handler.numeric_month();
            break;
        case 'd':
            handler.month_day();
            break;
        case 'e':
            handler.month_day(false);
            break;
        default:
            return Decorate::parse(it, end, handler);
        }

        return it + 2;
    }
};

} // namespace parser

template<class Decorate>
class parser_t {
    typedef typename Decorate::iterator_type iterator_type;

public:
    static void parse(const std::string& format, generator_handler_t& handler) {
        iterator_type it = format.begin();
        iterator_type end = format.end();

        while (it != end) {
            iterator_type p = std::find(it, end, '%');
            handler.partial_literal(it, p);

            if (std::distance(p, end) >= 2) {
                it = Decorate::parse(p, end, handler);
            } else {
                if (p != end) {
                    handler.partial_literal(p, end);
                }
                break;
            }
        }

        handler.end_partial_literal();
    }
};

class generator_factory_t {
public:
    static generator_t make(const std::string& pattern) {
        std::vector<generator_action_t> actions;
        generator_handler_t handler(actions);
        parser_t<parser::date<parser::time<parser::through_t>>>::parse(pattern, handler);
        return generator_t(std::move(actions));
    }
};

} // namespace datetime

} // namespace aux

} // namespace blackhole
