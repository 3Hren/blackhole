#include <blackhole/sink/stream.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(stream_t, Class) {
   sink::stream_t sink1(sink::stream_t::output_t::stdout);
   sink::stream_t sink2(sink::stream_t::output_t::stderr);
   UNUSED(sink1);
   UNUSED(sink2);
}

TEST(stream_t, IsThreadUnsafe) {
    static_assert(
        sink::thread_safety<
            sink::stream_t
        >::type::value == sink::thread::safety_t::unsafe,
        "`stream_t` sink must be thread unsafe"
    );
}

TEST(stream_t, StringConstructor) {
    sink::stream_t sink1("stdout");
    sink::stream_t sink2("stderr");
    UNUSED(sink1);
    UNUSED(sink2);
}

TEST(stream_t, StreamConstructor) {
    std::ostringstream stream;
    sink::stream_t sink(stream);
    sink.consume("test message");
    EXPECT_EQ("test message\n", stream.str());
}

TEST(stream_t, CanConsumeLogMessage) {
    sink::stream_t sink(sink::stream_t::output_t::stdout);
    sink.consume("test message for stream sink");
}
