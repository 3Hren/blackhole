#include <boost/lexical_cast.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/container/generation/vector_tie.hpp>
#include <boost/spirit/home/qi/auxiliary/eoi.hpp>
#include <boost/spirit/home/qi/auxiliary/eps.hpp>
#include <boost/spirit/home/qi/char/char.hpp>
#include <boost/spirit/home/qi/directive/as.hpp>
#include <boost/spirit/home/qi/nonterminal/error_handler.hpp>
#include <boost/spirit/home/qi/nonterminal/grammar.hpp>
#include <boost/spirit/home/qi/nonterminal/rule.hpp>
#include <boost/spirit/home/qi/operator/alternative.hpp>
#include <boost/spirit/home/qi/operator/difference.hpp>
#include <boost/spirit/home/qi/operator/expect.hpp>
#include <boost/spirit/home/qi/operator/kleene.hpp>
#include <boost/spirit/home/qi/operator/list.hpp>
#include <boost/spirit/home/qi/operator/and_predicate.hpp>
#include <boost/spirit/home/qi/operator/not_predicate.hpp>
#include <boost/spirit/home/qi/operator/optional.hpp>
#include <boost/spirit/home/qi/operator/permutation.hpp>
#include <boost/spirit/home/qi/operator/plus.hpp>
#include <boost/spirit/home/qi/operator/sequence.hpp>
#include <boost/spirit/home/qi/string/lit.hpp>

#include <blackhole/detail/formatter/string/error.hpp>
#include <blackhole/detail/formatter/string/grammar.hpp>
#include <blackhole/detail/formatter/string/grammar.inl.hpp>

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace formatter {
namespace string {
namespace {

namespace qi = boost::spirit::qi;

/// EBNF.
/// Grammar    ::= '{' Prefix? '...' Suffix? (':' Extension* FormatSpec?)? '}'
/// Extension  ::= '{' (((Any -':')* ':' [ps] '}' ('}' (Any - ':')* ':' [ps] '}')*))
/// Pattern    ::= '{' (PatternLiteral | Name | Value)* ':p}'
/// FormatSpec ::= [>^<]? [0-9]* [su]
struct grammar_t : public boost::spirit::qi::grammar<std::string::iterator, grammar_result_t()> {
    typedef std::string::iterator iterator_type;
    typedef grammar_result_t result_type;

    boost::spirit::qi::rule<iterator_type, grammar_result_t()> grammar;
    boost::spirit::qi::rule<iterator_type, std::string()> pattern;
    boost::spirit::qi::rule<iterator_type, std::string()> separator;
    boost::spirit::qi::rule<iterator_type, std::string()> spec;
    boost::spirit::qi::rule<iterator_type, char()> align;
    boost::spirit::qi::rule<iterator_type, std::string()> width;
    boost::spirit::qi::rule<iterator_type, char()> type;

    grammar_t();
};

struct optional_grammar_t : public boost::spirit::qi::grammar<std::string::iterator, optional_result_t()> {
    typedef std::string::iterator iterator_type;
    typedef optional_result_t result_type;

    boost::spirit::qi::rule<iterator_type, optional_result_t()> grammar;
    boost::spirit::qi::rule<iterator_type, std::string()> otherwise;
    boost::spirit::qi::rule<iterator_type, std::string()> spec;
    boost::spirit::qi::rule<iterator_type, char()> fill;
    boost::spirit::qi::rule<iterator_type, char()> align;
    boost::spirit::qi::rule<iterator_type, char()> alt;
    boost::spirit::qi::rule<iterator_type, char()> zero;
    boost::spirit::qi::rule<iterator_type, std::string()> width;
    boost::spirit::qi::rule<iterator_type, std::string()> precision;
    boost::spirit::qi::rule<iterator_type, char()> type;

    optional_grammar_t();
};

optional_grammar_t::optional_grammar_t() :
    optional_grammar_t::base_type(grammar)
{
    grammar %= qi::eps > qi::lit('{') > (
          '}'
        | ':' >> otherwise >> spec >> '}'
    ) > qi::eoi;
    otherwise %= qi::lit('{') >> (*(qi::char_ - (qi::lit(":default}") >> !qi::lit('}'))) >> ":default}") % '}';
    spec  %= -fill >> -align >> -alt >> -zero >> -width >> -precision >> -type;
    fill  %= qi::char_ >> &align;
    align %= qi::char_("<^>");
    alt   %= qi::char_('#');
    zero  %= qi::char_('0');
    width %= +qi::digit;
    precision %= qi::char_('.') >> +qi::digit;
    type  %= qi::char_("sdfx");
}

namespace tag {

using string::name;
using string::value;

}  // namespace tag

struct pattern_grammar_t :
    public boost::spirit::qi::grammar<std::string::iterator, std::vector<ph::leftover_t::token_t>()>
{
    typedef std::string::iterator iterator_type;
    typedef std::vector<ph::leftover_t::token_t> result_type;

    boost::spirit::qi::rule<iterator_type, std::vector<ph::leftover_t::token_t>()> grammar;
    boost::spirit::qi::rule<iterator_type, ph::attribute<tag::name>()> name;
    boost::spirit::qi::rule<iterator_type, ph::attribute<tag::value>()> value;
    boost::spirit::qi::rule<iterator_type, literal_t()> lit;
    boost::spirit::qi::rule<iterator_type, char()> lbrace;
    boost::spirit::qi::rule<iterator_type, char()> rbrace;
    boost::spirit::qi::rule<iterator_type, std::string()> spec;
    boost::spirit::qi::rule<iterator_type, char()> align;
    boost::spirit::qi::rule<iterator_type, std::string()> width;
    boost::spirit::qi::rule<iterator_type, char()> type;

    pattern_grammar_t();
};

grammar_t::grammar_t() :
    grammar_t::base_type(grammar)
{
    // See https://docs.python.org/2/library/string.html#format-specification-mini-language for
    // more information about the spec subset grammar.
    grammar    %= qi::eps > qi::lit('{') > (
          '}'
        | ':' >> -(pattern ^ separator) >> spec >> '}'
    ) > qi::eoi;
    pattern    %= qi::lit('{') >> (*(qi::char_ - (qi::lit(":p}") >> !qi::lit('}'))) >> ":p}") % '}';
    separator  %= qi::lit('{') >> (*(qi::char_ - (qi::lit(":s}") >> !qi::lit('}'))) >> ":s}") % '}';
    spec       %= -align >> -width >> type;
    align      %= qi::char_("<^>");
    width      %= +qi::digit;
    type       %= qi::char_('s');
}

pattern_grammar_t::pattern_grammar_t() :
    pattern_grammar_t::base_type(grammar)
{
    grammar    %= *(name | value | lit) >> qi::eoi;
    name       %= qi::as_string[qi::lit("{name") >> -(qi::lit(':') >> spec) >> '}'];
    value      %= qi::as_string[qi::lit("{value") >> -(qi::lit(':') >> spec) >> '}'];
    lit        %= qi::as_string[+(
          (qi::char_ - "{" - "{{" - "}" - "}}")
        | lbrace
        | rbrace
    ) >> qi::eps];
    lbrace      = qi::lit('{') >> qi::char_("{");
    rbrace      = qi::lit('}') >> qi::char_("}");
    spec       %= -align >> -width >> type;
    align      %= qi::char_("<^>");
    width      %= +qi::digit;
    type       %= qi::char_("sd");
}

template<typename G>
auto parse(std::string pattern) -> typename G::result_type {
    auto it = pattern.begin();
    const auto end = pattern.end();
    G grammar;
    typename G::result_type result;

    const auto parsed = [&]() -> bool {
        try {
            return boost::spirit::qi::parse(it, end, grammar, result);
        } catch (const boost::spirit::qi::expectation_failure<typename G::iterator_type>& err) {
            const auto pos = std::distance(pattern.begin(), err.last);
            throw parser_error_t(static_cast<std::size_t>(pos), pattern, "malformed input pattern");
        }
    }();

    if (parsed && (it == end)) {
        return result;
    }

    // TODO: More verbose message.
    throw std::runtime_error("NIY");
}

}  // namespace

auto parse(std::string pattern) -> grammar_result_t {
    return parse<grammar_t>(std::move(pattern));
}

auto parse_pattern(std::string pattern) -> std::vector<ph::leftover_t::token_t> {
    return parse<pattern_grammar_t>(std::move(pattern));
}

auto parse_optional(const std::string& name, const std::string& pattern) -> ph::generic<optional> {
    ph::generic<optional> token(name);

    const auto result = parse<optional_grammar_t>(pattern);
    token.spec = "{:" + result.spec + "}";

    try {
        token.otherwise = boost::lexical_cast<std::int64_t>(result.extension.otherwise.get());
        return token;
    } catch (const boost::bad_lexical_cast&) {}

    try {
        token.otherwise = boost::lexical_cast<double>(result.extension.otherwise.get());
        return token;
    } catch (const boost::bad_lexical_cast&) {}

    token.otherwise = result.extension.otherwise.get();
    return token;
}

auto parse_leftover(const std::string& pattern) -> ph::leftover_t {
    const auto r = parse(pattern);

    ph::leftover_t result;
    result.spec = "{:" + r.spec + "}";

    if (auto pattern = r.pattern()) {
        result.tokens = parse_pattern(*pattern);
    }

    if (auto separator = r.separator()) {
        result.separator = *separator;
    }

    return result;
}

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace v1
}  // namespace blackhole
