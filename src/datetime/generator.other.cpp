#if __APPLE__

#include "blackhole/detail/datetime/other/generator.hpp"

#include <ctime>
#include <functional>
#include <limits>
#include <locale>
#include <vector>

#include <boost/assert.hpp>

#include "blackhole/extensions/format.hpp"

#include "blackhole/detail/datetime/other/stream.hpp"

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace datetime {

namespace {

typedef fmt::MemoryWriter writer_type;
typedef ostream_adapter<writer_type> stream_type;

}  // namespace

struct context_t {
    // TODO: Get rid of intermediate stream for facets. Its construction consumes ~70ns, which is
    // comparable with other time required to format timestamps.
    stream_type wr;
    std::tm tm;
    std::uint64_t usec;
    const std::vector<std::string>& literals;
    std::vector<std::string>::const_iterator it;

    context_t(writer_type& wr, std::tm tm, std::uint64_t usec, const std::vector<std::string>& literals):
        wr(wr),
        tm(tm),
        usec(usec),
        literals(literals),
        it(std::begin(this->literals))
    {}
};

namespace {

template<std::size_t length, char filler = '0'>
inline auto fill(int value, char* buffer) -> void {
    std::size_t digits = 0;
    do {
        buffer[length - 1 - digits] = (value % 10) + '0';
        value /= 10;
        ++digits;
    } while (value);

    std::memset(buffer, filler, length - digits);
}

template<std::size_t length, char filler = '0'>
inline void fill(stream_type& wr, int value) {
    char buffer[std::numeric_limits<uint32_t>::digits10 + 2];
    fill<length, filler>(value, buffer);
    wr << fmt::StringRef(buffer, length);
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

    std::use_facet<facet_type>(context.wr.getloc())
        .put(iter_type(context.wr), context.wr, ' ', &context.tm, Format);
}

namespace year {

inline void full(context_t& context) {
    fill<4>(context.wr, context.tm.tm_year + 1900);
}

inline void last_two_digits(context_t& context) {
    fill<2>(context.wr, context.tm.tm_year % 100);
}

inline void first_two_digits(context_t& context) {
    fill<2>(context.wr, context.tm.tm_year / 100 + 19);
}

} // namespace year

namespace month {

inline void numeric(context_t& context) {
    fill<2>(context.wr, context.tm.tm_mon + 1);
}

} // namespace month

namespace week {

template<days_t firstweekday>
inline void year(context_t& context);

template<>
inline void year<days_t::sunday>(context_t& context) {
    fill<2>(context.wr, (context.tm.tm_yday - context.tm.tm_wday + 7) / 7);
}

template<>
inline void year<days_t::monday>(context_t& context) {
    fill<2>(context.wr, (context.tm.tm_yday - context.tm.tm_wday + (context.tm.tm_wday ? 8 : 1)) / 7);
}

} // namespace week

namespace day {

namespace year {

inline void numeric(context_t& context) {
    fill<3>(context.wr, context.tm.tm_yday + 1);
}

} // namespace year

namespace month {

template<char filler = '0'>
inline void numeric(context_t& context) {
    fill<2, filler>(context.wr, context.tm.tm_mday);
}

} // namespace month

} // namespace day

namespace time {

namespace hour {

inline void h24(context_t& context) {
    fill<2>(context.wr, context.tm.tm_hour);
}

inline void h12(context_t& context) {
    const int mod = context.tm.tm_hour % 12;
    fill<2>(context.wr, mod == 0 ? 12 : mod);
}

} // namespace hour

namespace minute {

inline void normal(context_t& context) {
    fill<2>(context.wr, context.tm.tm_min);
}

} // namespace minute

namespace second {

inline void normal(context_t& context) {
    fill<2>(context.wr, context.tm.tm_sec);
}

} // namespace second

namespace usecond {

inline void normal(context_t& context) {
    fill<6>(context.wr, static_cast<int>(context.usec));
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

    std::use_facet<facet_type>(context.wr.getloc())
        .put(iter_type(context.wr), context.wr, ' ', &context.tm, 'a');
    context.wr << ' ';
    std::use_facet<facet_type>(context.wr.getloc())
        .put(iter_type(context.wr), context.wr, ' ', &context.tm, 'b');
    context.wr << ' ';
    day::month::numeric(context);
    context.wr << ' ';
    time::hour::h24(context);
    context.wr << ':';
    time::minute::normal(context);
    context.wr << ':';
    time::second::normal(context);
    context.wr << ' ';
    year::full(context);
}

inline void date_mdy(context_t& context) {
    month::numeric(context);
    context.wr << '/';
    day::month::numeric(context);
    context.wr << '/';
    year::last_two_digits(context);
}

inline void date_ISO8601(context_t& context) {
    year::full(context);
    context.wr << '-';
    month::numeric(context);
    context.wr << '-';
    day::month::numeric(context);
}

inline void time_ISO8601(context_t& context) {
    time::hour::h24(context);
    context.wr << ':';
    time::minute::normal(context);
    context.wr << ':';
    time::second::normal(context);
}

} // namespace visit

inline void literal_generator_t(context_t& context) {
    context.wr << *context.it;
    ++context.it;
}

class generator_handler_t {
    typedef std::string::const_iterator iterator_type;

    std::vector<token_t>& tokens;
    std::vector<std::string>& literals;

    std::string litbuffer;

public:
    generator_handler_t(std::vector<token_t>& tokens, std::vector<std::string>& literals) :
        tokens(tokens),
        literals(literals)
    {}

    virtual ~generator_handler_t() {}

    virtual void partial_literal(iterator_type begin, iterator_type end) {
        litbuffer.append(begin, end);
    }

    virtual void end_partial_literal() {
        if (!litbuffer.empty()) {
            literal(litbuffer);
            litbuffer.clear();
        }
    }

    virtual void literal(const std::string& literal) {
        tokens.push_back(&literal_generator_t);
        literals.push_back(literal);
    }

    virtual void placeholder(const std::string& value) {
        literal(value);
    }

    virtual void full_year() {
        end_partial_literal();
        tokens.push_back(&visit::year::full);
    }

    virtual void short_year() {
        end_partial_literal();
        tokens.push_back(&visit::year::last_two_digits);
    }

    virtual void short_year_first_two_digits() {
        end_partial_literal();
        tokens.push_back(&visit::year::first_two_digits);
    }

    virtual void numeric_month() {
        end_partial_literal();
        tokens.push_back(&visit::month::numeric);
    }

    virtual void abbreviate_month() {
        end_partial_literal();
        tokens.push_back(&visit::localized<'b'>);
    }

    virtual void full_month() {
        end_partial_literal();
        tokens.push_back(&visit::localized<'B'>);
    }

    virtual void year_week(days_t firstweekday) {
        end_partial_literal();
        if (firstweekday == days_t::sunday) {
            tokens.push_back(&visit::week::year<days_t::sunday>);
        } else {
            tokens.push_back(&visit::week::year<days_t::monday>);
        }
    }

    virtual void year_day() {
        end_partial_literal();
        tokens.push_back(&visit::day::year::numeric);
    }

    virtual void month_day(bool has_leading_zero = true) {
        end_partial_literal();
        if (has_leading_zero) {
            tokens.push_back(&visit::day::month::numeric<'0'>);
        } else {
            tokens.push_back(&visit::day::month::numeric<' '>);
        }
    }

    virtual void abbreviate_weekday() {
        end_partial_literal();
        tokens.push_back(&visit::localized<'a'>);
    }

    virtual void full_weekday() {
        end_partial_literal();
        tokens.push_back(&visit::localized<'A'>);
    }

    virtual void hours() {
        end_partial_literal();
        tokens.push_back(&visit::time::hour::h24);
    }

    virtual void hours12() {
        end_partial_literal();
        tokens.push_back(&visit::time::hour::h12);
    }

    virtual void minute() {
        end_partial_literal();
        tokens.push_back(&visit::time::minute::normal);
    }

    virtual void second() {
        end_partial_literal();
        tokens.push_back(&visit::time::second::normal);
    }

    virtual void usecond() {
        end_partial_literal();
        tokens.push_back(&visit::time::usecond::normal);
    }

    virtual void ampm() {
        end_partial_literal();
        tokens.push_back(&visit::localized<'p'>);
    }

    virtual void utc_offset() {
        end_partial_literal();
        tokens.push_back(&visit::utc_offset);
    }

    virtual void timezone() {
        end_partial_literal();
        tokens.push_back(&visit::localized<'Z'>);
    }

    virtual void standard_date_time() {
        end_partial_literal();
        tokens.push_back(&visit::standard);
    }

    virtual void date_mdy() {
        end_partial_literal();
        tokens.push_back(&visit::date_mdy);
    }

    virtual void date_ISO8601() {
        end_partial_literal();
        tokens.push_back(&visit::date_ISO8601);
    }

    virtual void time_ISO8601() {
        end_partial_literal();
        tokens.push_back(&visit::time_ISO8601);
    }
};

namespace parser {

class through_t {
public:
    typedef std::string::const_iterator iterator_type;

    static iterator_type parse(iterator_type it, iterator_type end, generator_handler_t& handler) {
        BOOST_ASSERT(it != end && it + 1 != end);
        (void)end;

        handler.placeholder(std::string(it, it + 2));
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

}  // namespace

generator_t::generator_t(std::vector<token_t> tokens, std::vector<std::string> literals) :
    tokens(std::move(tokens)),
    literals(std::move(literals))
{}

template<typename Stream>
auto
generator_t::operator()(Stream& stream, const std::tm& tm, std::uint64_t usec) const -> void {
    context_t context{stream, tm, usec, literals};

    for (const auto& token : tokens) {
        token(context);
    }
}

auto make_generator(const std::string& pattern) -> generator_t {
    std::vector<token_t> tokens;
    std::vector<std::string> literals;

    generator_handler_t handler(tokens, literals);

    parser_t<parser::date<parser::time<parser::common<parser::through_t>>>>::parse(pattern, handler);

    return generator_t(std::move(tokens), std::move(literals));
}

template
auto generator_t::operator()<writer_type>(writer_type&, const std::tm&, std::uint64_t) const -> void;

}  // namespace datetime
}  // namespace detail
}  // namespace v1
}  // namespace blackhole

#endif
