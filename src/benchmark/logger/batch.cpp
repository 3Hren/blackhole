#include <boost/preprocessor.hpp>

#include <epicmeter/benchmark.hpp>

#include <blackhole/formatter/json.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/frontend/files.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/macro.hpp>
#include <blackhole/sink/null.hpp>
#include <blackhole/sink/files.hpp>

#include "../util.hpp"

namespace { enum level_t { debug, info }; }

#define MESSAGE_S0  "Le message"
#define MESSAGE_S1  "Le message: %s"
#define MESSAGE_S2  "Le message: [%d] %s"
#define MESSAGE_S3  "Le message: [%d] %s - %s"

#define MESSAGE_M0  "Something bad is going on but I can handle it now!"
#define MESSAGE_M1  "Something bad is going on but I can handle it now: %s!"
#define MESSAGE_M2  "Something bad is going on but I can handle it now: [%d] %s!"
#define MESSAGE_M3  "Something bad is going on but I can handle it now: [%d] %s - %s!"

#define MESSAGE_L0 "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore"
#define MESSAGE_L1 "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore: %s"
#define MESSAGE_L2 "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore: [%d] %s"
#define MESSAGE_L3 "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore: [%d] %s - %s"

//! ============================================================================
#define CAT(x, y) CAT_I(x, y)
#define CAT_I(x, y) x ## y

#define APPLY(macro, args) APPLY_I(macro, args)
#define APPLY_I(macro, args) macro args

#define STRIP_PARENS(x) EVAL((STRIP_PARENS_I x), x)
#define STRIP_PARENS_I(...) 1,1

#define EVAL(test, x) EVAL_I(test, x)
#define EVAL_I(test, x) MAYBE_STRIP_PARENS(TEST_ARITY test, x)

#define TEST_ARITY(...) APPLY(TEST_ARITY_I, (__VA_ARGS__, 2, 1))
#define TEST_ARITY_I(a,b,c,...) c

#define MAYBE_STRIP_PARENS(cond, x) MAYBE_STRIP_PARENS_I(cond, x)
#define MAYBE_STRIP_PARENS_I(cond, x) CAT(MAYBE_STRIP_PARENS_, cond)(x)

#define MAYBE_STRIP_PARENS_1(x) x
#define MAYBE_STRIP_PARENS_2(x) APPLY(MAYBE_STRIP_PARENS_2_I, x)
#define MAYBE_STRIP_PARENS_2_I(...) __VA_ARGS__
//! ============================================================================

#define SUITE(FORMATTER, SINK, FILTER_DESC, FILTER_ACT, msg, MSG, ARGS, attrs, ATTRIBUTES) \
BENCHMARK(Logger_, (L: Verbose, \
    F: BOOST_PP_TUPLE_ELEM(3, 0, FORMATTER), \
    S: BOOST_PP_TUPLE_ELEM(3, 0, SINK), \
    R: FILTER_DESC, \
    M: msg, \
    A: attrs))\
{ \
    static auto log = initialize< \
        blackhole::verbose_logger_t<level_t>, \
        BOOST_PP_TUPLE_ELEM(3, 1, FORMATTER), \
        BOOST_PP_TUPLE_ELEM(3, 1, SINK) \
    >()\
        .formatter BOOST_PP_TUPLE_ELEM(3, 2, FORMATTER) \
        .sink BOOST_PP_TUPLE_ELEM(3, 2, SINK) \
        .log(level_t::debug) \
        .mod(FILTER_ACT) \
        .get(); \
    \
    BH_LOG(log, level_t::debug, MSG ARGS) ATTRIBUTES; \
}

// Transform all the shit to the message placeholder placed in benchmark name.
#define MESSAGE_PH(log, fmt, snk, flt, msg, msgargs, attrs) \
    BOOST_PP_SEQ_CAT((MESSAGE_)(msg)(_)(BOOST_PP_TUPLE_ELEM(2, 0, msgargs)))

#define MESSAGE_VAR(log, fmt, snk, flt, msg, msgargs, attrs) \
    BOOST_PP_CAT(BOOST_PP_CAT(MESSAGE_, msg), BOOST_PP_TUPLE_ELEM(2, 0, msgargs))

#define MESSAGE_ARGS(log, fmt, snk, flt, msg, msgargs, attrs) \
    STRIP_PARENS(BOOST_PP_TUPLE_ELEM(2, 1, msgargs))

#define ATTRIBUTES_PH(log, fmt, snk, flt, msg, msgargs, attrs) \
    BOOST_PP_TUPLE_ELEM(2, 0, attrs)

#define ATTRIBUTES_ARGS(log, fmt, snk, flt, msg, msgargs, attrs) \
    BOOST_PP_TUPLE_ELEM(1, 0, BOOST_PP_TUPLE_ELEM(2, 1, attrs))

#define SUITE0(r, product) \
    SUITE( \
        BOOST_PP_SEQ_ELEM(1, product), /* Formatter */ \
        BOOST_PP_SEQ_ELEM(2, product), /* Sink */ \
        BOOST_PP_TUPLE_ELEM(2, 0, BOOST_PP_SEQ_ELEM(3, product)), \
        BOOST_PP_TUPLE_ELEM(1, 0, BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_SEQ_ELEM(3, product))), \
        MESSAGE_PH BOOST_PP_SEQ_TO_TUPLE(product), \
        MESSAGE_VAR BOOST_PP_SEQ_TO_TUPLE(product), \
        MESSAGE_ARGS BOOST_PP_SEQ_TO_TUPLE(product), \
        ATTRIBUTES_PH BOOST_PP_SEQ_TO_TUPLE(product), \
        ATTRIBUTES_ARGS BOOST_PP_SEQ_TO_TUPLE(product) \
    )

#define LOGGER_SEQ ()
#define FORMATTER_SEQ \
    ((String, blackhole::formatter::string_t, ("[%(timestamp)s]: %(message)s"))) \
    ((Json,   blackhole::formatter::json_t,   ()))

#define SINK_SEQ \
    ((Null,  blackhole::sink::null_t,    ())) \
    ((Files, blackhole::sink::files_t<>, (blackhole::sink::files::config_t<>("blackhole.log"))))

#define MESSAGE_SEQ (S)(M)(L)

#define MESSAGE_ARGS_SEQ \
    ((0, ())) \
    ((1, ( , "okay"))) \
    ((2, ( , 42, "okay"))) \
    ((3, ( , 42, "okay", "description")))

#define ATTRIBUTES_SEQ \
    ((0, ())) \
    ((1, (("id", 42)))) \
    ((2, (("id", 42, "info", "le string")))) \
    ((3, (("id", 42, "info", "le string", "method", "POST"))))

#define FILTER_SEQ \
    ((PASS, ())) \
    ((FAIL, (std::bind(&filter_by::verbosity<level_t>, std::placeholders::_1, level_t::info))))

BOOST_PP_SEQ_FOR_EACH_PRODUCT(SUITE0, \
    (LOGGER_SEQ) \
    (FORMATTER_SEQ)(SINK_SEQ)(FILTER_SEQ) \
    (MESSAGE_SEQ)(MESSAGE_ARGS_SEQ)(ATTRIBUTES_SEQ))
