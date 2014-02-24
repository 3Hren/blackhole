#include <ctime>
#include <functional>

#include <boost/assert.hpp>
#include <boost/range.hpp>

#include "global.hpp"

struct context_t {
    std::ostringstream& stream;
    std::tm tm;
};

namespace aux {

inline uint digits(int value) {
    if (value == 0) {
        return 1;
    }

    uint digits = 0;
    while (value) {
        value /= 10;
        digits++;
    }

    return digits;
}

} // namespace aux

inline void fill(std::ostringstream& stream, int value, int length, char filler = '0') {
    const int digits = aux::digits(value);
    const int gap = length - digits;
    BOOST_ASSERT(gap >= 0);
    for (int i = 0; i < gap; ++i) {
        stream << filler;
    }
    stream << value;
}

namespace visit {

namespace year {

inline void full(context_t& context) {
    fill(context.stream, context.tm.tm_year, 4);
}

inline void normal(context_t& context) {
    fill(context.stream, context.tm.tm_year % 100, 2);
}

} // namespace year

namespace month {

inline void numeric(context_t& context) {
    fill(context.stream, context.tm.tm_mon, 2);
}

} // namespace month

namespace day {

namespace month {

template<char Filler = '0'>
inline void numeric(context_t& context) {
    fill(context.stream, context.tm.tm_mday, 2, Filler);
}

} // namespace month

} // namespace day

namespace hour {

inline void h24(context_t& context) {
    fill(context.stream, context.tm.tm_hour, 2);
}

inline void h12(context_t& context) {
    const uint mod = context.tm.tm_hour % 12;
    fill(context.stream, mod == 0 ? 12 : mod, 2);
}

} // namespace hour

namespace time {

namespace minute {

inline void normal(context_t& context) {
    fill(context.stream, context.tm.tm_min, 2);
}

} // namespace minute

namespace second {

inline void normal(context_t& context) {
    fill(context.stream, context.tm.tm_sec, 2);
}

} // namespace second

} // namespace time

} // namespace visit

struct literal_generator_t {
    std::string literal;

    void operator()(context_t& context) const {
        context.stream << literal;
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
        context_t context { stream, tm };

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

using namespace datetime;

class generator_test_case_t : public Test {
protected:
    std::tm tm;

    void SetUp() {
        tm = std::tm();
    }

    std::string generate(const std::string& pattern) {
        std::ostringstream stream;
        generator_t generator = generator_factory_t::make(pattern);
        generator(stream, tm);
        return stream.str();
    }
};

TEST_F(generator_test_case_t, FullYear) {
    tm.tm_year = 2014;
    EXPECT_EQ("2014", generate("%Y"));
}

TEST_F(generator_test_case_t, ShortYear) {
    tm.tm_year = 2014;
    EXPECT_EQ("14", generate("%y"));
}

TEST_F(generator_test_case_t, ShortYearWithZeroPrefix) {
    tm.tm_year = 2004;
    EXPECT_EQ("04", generate("%y"));
}

TEST_F(generator_test_case_t, ShortYearMinValue) {
    tm.tm_year = 2000;
    EXPECT_EQ("00", generate("%y"));
}

TEST_F(generator_test_case_t, FullYearWithSuffixLiteral) {
    tm.tm_year = 2014;
    EXPECT_EQ("2014-", generate("%Y-"));
}

TEST_F(generator_test_case_t, FullYearWithPrefixLiteral) {
    tm.tm_year = 2014;
    EXPECT_EQ("-2014", generate("-%Y"));
}

TEST_F(generator_test_case_t, FullYearWithPrefixAndSuffixLiteral) {
    tm.tm_year = 2014;
    EXPECT_EQ("-2014-", generate("-%Y-"));
}

TEST_F(generator_test_case_t, NumericMonth) {
    tm.tm_mon = 2;
    EXPECT_EQ("02", generate("%m"));
}

TEST_F(generator_test_case_t, NumericDayOfMonth) {
    tm.tm_mday = 23;
    EXPECT_EQ("23", generate("%d"));
}

TEST_F(generator_test_case_t, NumericDayOfMonthWithSingleDigit) {
    tm.tm_mday = 6;
    EXPECT_EQ("06", generate("%d"));
}

TEST_F(generator_test_case_t, ShortNumericDayOfMonth) {
    tm.tm_mday = 23;
    EXPECT_EQ("23", generate("%e"));
}

TEST_F(generator_test_case_t, FullDayHour) {
    tm.tm_hour = 11;
    EXPECT_EQ("11", generate("%H"));
}

TEST_F(generator_test_case_t, FullDayHourWithSingleDigit) {
    tm.tm_hour = 0;
    EXPECT_EQ("00", generate("%H"));
}

TEST_F(generator_test_case_t, HalfDayHour) {
    //!@note: tm_hour is treated as [0; 23], so 00:30 will be 12:30.
    tm.tm_hour = 11;
    EXPECT_EQ("11", generate("%I"));
}

TEST_F(generator_test_case_t, HalfDayHourWithSingleDigit) {
    tm.tm_hour = 6;
    EXPECT_EQ("06", generate("%I"));
}

TEST_F(generator_test_case_t, HalfDayHourWithOverflow) {
    tm.tm_hour = 13;
    EXPECT_EQ("01", generate("%I"));
}

TEST_F(generator_test_case_t, HalfDayHourLowerBorderCase) {
    tm.tm_hour = 0;
    EXPECT_EQ("12", generate("%I"));
}

TEST_F(generator_test_case_t, HalfDayHourUpperBorderCase) {
    tm.tm_hour = 12;
    EXPECT_EQ("12", generate("%I"));
}

TEST_F(generator_test_case_t, Minute) {
    tm.tm_min = 30;
    EXPECT_EQ("30", generate("%M"));
}

TEST_F(generator_test_case_t, MinuteLowerBound) {
    tm.tm_min = 00;
    EXPECT_EQ("00", generate("%M"));
}

TEST_F(generator_test_case_t, MinuteUpperBound) {
    tm.tm_min = 59;
    EXPECT_EQ("59", generate("%M"));
}

TEST_F(generator_test_case_t, Second) {
    tm.tm_sec = 30;
    EXPECT_EQ("30", generate("%S"));
}

TEST_F(generator_test_case_t, SecondLowerBound) {
    tm.tm_sec = 0;
    EXPECT_EQ("00", generate("%S"));
}

TEST_F(generator_test_case_t, SecondUpperBound) {
    tm.tm_sec = 60;
    EXPECT_EQ("60", generate("%S"));
}

TEST_F(generator_test_case_t, ComplexFormatting) {
    tm.tm_year = 2014;
    tm.tm_mon = 2;
    tm.tm_mday = 23;
    tm.tm_hour = 12;
    tm.tm_min = 20;
    tm.tm_sec = 30;
    EXPECT_EQ("2014-02-23 12:20:30", generate("%Y-%m-%d %H:%M:%S"));
}
