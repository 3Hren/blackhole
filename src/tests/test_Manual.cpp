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
        actions.push_back(&visit::year::full);
    }

    virtual void short_year() {
        actions.push_back(&visit::year::normal);
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
};

template<class Decorate>
class time {
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
        parser_t<parser::date<parser::through_t>>::parse(pattern, handler);
        return generator_t(std::move(actions));
    }
};

} // namespace datetime

using namespace datetime;

TEST(generator_t, FullYear) {
    generator_t generator = generator_factory_t::make("%Y");
    std::ostringstream stream;
    std::tm tm;
    tm.tm_year = 2014;
    generator(stream, tm);
    EXPECT_EQ("2014", stream.str());
}

TEST(generator_t, ShortYear) {
    generator_t generator = generator_factory_t::make("%y");
    std::ostringstream stream;
    std::tm tm;
    tm.tm_year = 2014;
    generator(stream, tm);
    EXPECT_EQ("14", stream.str());
}

TEST(generator_t, ShortYearWithZeroPrefix) {
    generator_t generator = generator_factory_t::make("%y");
    std::ostringstream stream;
    std::tm tm;
    tm.tm_year = 2004;
    generator(stream, tm);
    EXPECT_EQ("04", stream.str());
}

TEST(generator_t, ShortYearMinValue) {
    generator_t generator = generator_factory_t::make("%y");
    std::ostringstream stream;
    std::tm tm;
    tm.tm_year = 2000;
    generator(stream, tm);
    EXPECT_EQ("00", stream.str());
}

TEST(generator_t, FullYearWithSuffixLiteral) {
    generator_t generator = generator_factory_t::make("%Y-");
    std::ostringstream stream;
    std::tm tm;
    tm.tm_year = 2014;
    generator(stream, tm);
    EXPECT_EQ("2014-", stream.str());
}
