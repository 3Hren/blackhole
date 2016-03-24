#include <string>

#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/optional/optional.hpp>
#include <boost/spirit/home/qi/nonterminal/grammar.hpp>
#include <boost/spirit/home/qi/nonterminal/rule.hpp>
#include <boost/variant/variant.hpp>

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace formatter {
namespace string {

/// Represents parser grammar result.
/// Given: `{...:{{name}={value}:p}{, :s}>50s}`.
///     Spec: `>50s`.
///     Extension:
///         Pattern: `{name}={value}`.
///         Separator: `, `.
struct grammar_result_t {
    struct extension_t {
        boost::optional<std::string> pattern;
        boost::optional<std::string> separator;
    };

    extension_t extension;
    std::string spec;

    auto pattern() const -> boost::optional<std::string> {
        return extension.pattern;
    }

    auto separator() const -> boost::optional<std::string> {
        return extension.separator;
    }
};

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace v1
}  // namespace blackhole

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"

BOOST_FUSION_ADAPT_STRUCT(blackhole::detail::formatter::string::grammar_result_t::extension_t,
    (boost::optional<std::string>, pattern)
    (boost::optional<std::string>, separator)
)

BOOST_FUSION_ADAPT_STRUCT(blackhole::detail::formatter::string::grammar_result_t,
    (blackhole::detail::formatter::string::grammar_result_t::extension_t, extension)
    (std::string, spec)
)

#pragma clang diagnostic pop

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace formatter {
namespace string {

/// EBNF.
/// Grammar    ::= '{' Prefix? '...' Suffix? (':' Extension* FormatSpec?)? '}'
/// Extension  ::= '{' (((Any -':')* ':' [ps] '}' ('}' (Any - ':')* ':' [ps] '}')*))
/// Pattern    ::= '{' (PatternLiteral | Name | Value)* ':p}'
/// FormatSpec ::= [>^<]? [0-9]* [su]
struct grammar_t : public boost::spirit::qi::grammar<std::string::iterator, grammar_result_t()> {
    typedef std::string::iterator iterator_type;

    boost::spirit::qi::rule<iterator_type, grammar_result_t()> grammar;
    boost::spirit::qi::rule<iterator_type, std::string()> pattern;
    boost::spirit::qi::rule<iterator_type, std::string()> separator;
    boost::spirit::qi::rule<iterator_type, std::string()> spec;
    boost::spirit::qi::rule<iterator_type, char()> align;
    boost::spirit::qi::rule<iterator_type, std::string()> width;
    boost::spirit::qi::rule<iterator_type, char()> type;

    grammar_t();
};

namespace tag {

using string::name;
using string::value;

}  // namespace tag

struct pattern_grammar_t :
    public boost::spirit::qi::grammar<std::string::iterator, std::vector<ph::leftover_t::token_t>()>
{
    typedef std::string::iterator iterator_type;

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

auto parse(std::string pattern) -> grammar_result_t;
auto parse_pattern(std::string pattern) -> std::vector<ph::leftover_t::token_t>;

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace v1
}  // namespace blackhole
