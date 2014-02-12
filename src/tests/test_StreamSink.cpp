#include "Mocks.hpp"

namespace blackhole {

namespace sink {

class stream_t{
    std::ostream& stream;
public:
    enum class output_t {
        stdout,
        stderr
    };

    stream_t(output_t output) :
        stream(stream_factory_t::get(output))
    {}

    void consume(const std::string& message) {
        stream << message << std::endl;
    }

private:
    struct stream_factory_t {
        static std::ostream& get(output_t output) {
            switch (output) {
            case output_t::stdout:
                return std::cout;
            case output_t::stderr:
                return std::cerr;
            default:
                BOOST_ASSERT(false);
            }
        }
    };
};

} // namespace sink

} // namespace blackhole

TEST(stream_t, Class) {
    sink::stream_t sink(sink::stream_t::output_t::stdout);
    UNUSED(sink);
}

TEST(stream_t, CanConsumeLogMessage) {
    sink::stream_t sink(sink::stream_t::output_t::stdout);
    sink.consume("test message for stream sink");
}

// TEST(stream_t, StderrConstructor)
// TEST(stream_t, StringConstructor)
// TEST(stream_t, StdoutByDefaultIfStringConstructorFails)
