#pragma once

#include <ctime>
#include <functional>
#include <limits>
#include <locale>
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
    const std::tm tm;
    const suseconds_t usec;
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

enum class days_t {
    sunday,
    monday
};

namespace visit {

template<char Format>
inline void localized(context_t& context) {
    typedef std::time_put<char> facet_type;
    typedef typename facet_type::iter_type iter_type;
    std::use_facet<facet_type>(context.stream.getloc())
            .put(iter_type(context.stream), context.stream, ' ', &context.tm, Format);
    context.stream.flush();
}

namespace year {

inline void full(context_t& context) {
    fill(context.str, context.tm.tm_year + 1900, 4);
}

inline void last_two_digits(context_t& context) {
    fill(context.str, context.tm.tm_year % 100, 2);
}

inline void first_two_digits(context_t& context) {
    fill(context.str, context.tm.tm_year / 100 + 19, 2);
}

} // namespace year

namespace month {

inline void numeric(context_t& context) {
    fill(context.str, context.tm.tm_mon + 1, 2);
}

} // namespace month

namespace week {

template<days_t firstweekday>
inline void year(context_t& context);

template<>
inline void year<days_t::sunday>(context_t& context) {
    fill(context.str, (context.tm.tm_yday - context.tm.tm_wday + 7) / 7, 2);
}

template<>
inline void year<days_t::monday>(context_t& context) {
    fill(context.str, (context.tm.tm_yday - context.tm.tm_wday + (context.tm.tm_wday ? 8 : 1)) / 7, 2);
}

} // namespace week

namespace day {

namespace year {

inline void numeric(context_t& context) {
    fill(context.str, context.tm.tm_yday + 1, 3);
}

} // namespace year

namespace month {

template<char Filler = '0'>
inline void numeric(context_t& context) {
    fill(context.str, context.tm.tm_mday, 2, Filler);
}

} // namespace month

} // namespace day

namespace time {

namespace hour {

inline void h24(context_t& context) {
    fill(context.str, context.tm.tm_hour, 2);
}

inline void h12(context_t& context) {
    const uint mod = context.tm.tm_hour % 12;
    fill(context.str, mod == 0 ? 12 : mod, 2);
}

} // namespace hour

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

namespace usecond {

inline void normal(context_t& context) {
    fill(context.str, context.usec, 6);
}

} // namespace usecond

} // namespace time

inline void utc_offset(context_t& context) {
    // It's too dificult to reinvent this case depending on platform.
    // See GNUC strftime if you don't believe.
    localized<'z'>(context);
}

inline void standard(context_t& context) {
    typedef std::time_put<char> facet_type;
    typedef facet_type::iter_type iter_type;
    auto locale = context.stream.getloc();
    std::use_facet<facet_type>(locale)
            .put(iter_type(context.stream), context.stream, ' ', &context.tm, 'a');
    context.stream.flush();
    context.str.push_back(' ');
    std::use_facet<facet_type>(locale)
            .put(iter_type(context.stream), context.stream, ' ', &context.tm, 'b');
    context.stream.flush();
    context.str.push_back(' ');
    day::month::numeric(context);
    context.str.push_back(' ');
    time::hour::h24(context);
    context.str.push_back(':');
    time::minute::normal(context);
    context.str.push_back(':');
    time::second::normal(context);
    context.str.push_back(' ');
    year::full(context);
}

inline void date_mdy(context_t& context) {
    month::numeric(context);
    context.str.push_back('/');
    day::month::numeric(context);
    context.str.push_back('/');
    year::last_two_digits(context);
}

inline void date_ISO8601(context_t& context) {
    year::full(context);
    context.str.push_back('-');
    month::numeric(context);
    context.str.push_back('-');
    day::month::numeric(context);
}

inline void time_ISO8601(context_t& context) {
    time::hour::h24(context);
    context.str.push_back(':');
    time::minute::normal(context);
    context.str.push_back(':');
    time::second::normal(context);
}

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
    void operator()(Stream& stream, const std::tm& tm, suseconds_t usec = 0) const {
        context_t context { stream, *stream.rdbuf()->storage(), tm, usec };

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

    virtual ~generator_handler_t() {}

    virtual void partial_literal(iterator_type begin, iterator_type end) {
        m_literal.append(begin, end);
    }

    virtual void end_partial_literal() {
        if (!m_literal.empty()) {
            literal(boost::make_iterator_range(m_literal));
            m_literal.clear();
        }
    }

    virtual void literal(const std::string& lit) {
        actions.push_back(literal_generator_t { lit });
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
        actions.push_back(&visit::year::last_two_digits);
    }

    virtual void short_year_first_two_digits() {
        end_partial_literal();
        actions.push_back(&visit::year::first_two_digits);
    }

    virtual void numeric_month() {
        end_partial_literal();
        actions.push_back(&visit::month::numeric);
    }

    virtual void abbreviate_month() {
        end_partial_literal();
        actions.push_back(&visit::localized<'b'>);
    }

    virtual void full_month() {
        end_partial_literal();
        actions.push_back(&visit::localized<'B'>);
    }

    virtual void year_week(days_t firstweekday) {
        end_partial_literal();
        if (firstweekday == days_t::sunday) {
            actions.push_back(&visit::week::year<days_t::sunday>);
        } else {
            actions.push_back(&visit::week::year<days_t::monday>);
        }
    }

    virtual void year_day() {
        end_partial_literal();
        actions.push_back(&visit::day::year::numeric);
    }

    virtual void month_day(bool has_leading_zero = true) {
        end_partial_literal();
        if (has_leading_zero) {
            actions.push_back(&visit::day::month::numeric<'0'>);
        } else {
            actions.push_back(&visit::day::month::numeric<' '>);
        }
    }

    virtual void abbreviate_weekday() {
        end_partial_literal();
        actions.push_back(&visit::localized<'a'>);
    }

    virtual void full_weekday() {
        end_partial_literal();
        actions.push_back(&visit::localized<'A'>);
    }

    virtual void hours() {
        end_partial_literal();
        actions.push_back(&visit::time::hour::h24);
    }

    virtual void hours12() {
        end_partial_literal();
        actions.push_back(&visit::time::hour::h12);
    }

    virtual void minute() {
        end_partial_literal();
        actions.push_back(&visit::time::minute::normal);
    }

    virtual void second() {
        end_partial_literal();
        actions.push_back(&visit::time::second::normal);
    }

    virtual void usecond() {
        end_partial_literal();
        actions.push_back(&visit::time::usecond::normal);
    }

    virtual void ampm() {
        end_partial_literal();
        actions.push_back(&visit::localized<'p'>);
    }

    virtual void utc_offset() {
        end_partial_literal();
        actions.push_back(&visit::utc_offset);
    }

    virtual void timezone() {
        end_partial_literal();
        actions.push_back(&visit::localized<'Z'>);
    }

    virtual void standard_date_time() {
        end_partial_literal();
        actions.push_back(&visit::standard);
    }

    virtual void date_mdy() {
        end_partial_literal();
        actions.push_back(&visit::date_mdy);
    }

    virtual void date_ISO8601() {
        end_partial_literal();
        actions.push_back(&visit::date_ISO8601);
    }

    virtual void time_ISO8601() {
        end_partial_literal();
        actions.push_back(&visit::time_ISO8601);
    }
};

namespace parser {

class through_t {
public:
    typedef std::string::const_iterator iterator_type;

    static iterator_type parse(iterator_type it, iterator_type end, generator_handler_t& handler) {
        BOOST_ASSERT(it != end && it + 1 != end);
#ifdef BOOST_DISABLE_ASSERTS
        (void)end;
#endif
        handler.placeholder(boost::make_iterator_range(it, it + 2));
        return it + 2;
    }
};

template<class Decorate = through_t>
class common {
public:
    typedef typename Decorate::iterator_type iterator_type;

    static iterator_type parse(iterator_type it, iterator_type end, generator_handler_t& handler) {
        BOOST_ASSERT(it != end && it + 1 != end);
        switch (*(it + 1)) {
        case 'c':
            break;
        default:
            return Decorate::parse(it, end, handler);
        }
        return it + 2;
    }
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
        case 'f':
            handler.usecond();
            break;
        case 'p':
            handler.ampm();
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
        //! =========== YEAR SECTION ===========
        case 'Y':
            handler.full_year();
            break;
        case 'y':
            handler.short_year();
            break;
        case 'C':
            handler.short_year_first_two_digits();
            break;
        //! =========== MONTH SECTION ===========
        case 'm':
            handler.numeric_month();
            break;
        case 'b':
        case 'h':
            handler.abbreviate_month();
            break;
        case 'B':
            handler.full_month();
            break;
        //! =========== WEAK SECTION ===========
        case 'U':
            handler.year_week(days_t::sunday);
            break;
        case 'W':
            handler.year_week(days_t::monday);
            break;
        //! =========== DAY SECTION ===========
        case 'j':
            handler.year_day();
            break;
        case 'd':
            handler.month_day();
            break;
        case 'e':
            handler.month_day(false);
            break;
        case 'a':
            handler.abbreviate_weekday();
            break;
        case 'A':
            handler.full_weekday();
            break;
        //! =========== OTHER SECTION ===========
        case 'c':
            handler.standard_date_time();
            break;
        case 'D':
            handler.date_mdy();
            break;
        case 'F':
            handler.date_ISO8601();
            break;
        case 'T':
            handler.time_ISO8601();
            break;
        case 'z':
            handler.utc_offset();
            break;
        case 'Z':
            handler.timezone();
            break;
        case '%':
            handler.literal(std::string("%"));
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
        parser_t<parser::date<parser::time<parser::common<parser::through_t>>>>::parse(pattern, handler);
        return generator_t(std::move(actions));
    }
};

} // namespace datetime

} // namespace aux

} // namespace blackhole
