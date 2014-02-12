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

    stream_t(const std::string& output) :
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

        static std::ostream& get(const std::string& output) {
            output_t out = output_t::stdout;

            if (output == "stdout") {
                out = output_t::stdout;
            } else if (output == "stderr") {
                out = output_t::stderr;
            }

            return get(out);
        }
    };
};

} // namespace sink

} // namespace blackhole

TEST(stream_t, Class) {
   sink::stream_t sink1(sink::stream_t::output_t::stdout);
   sink::stream_t sink2(sink::stream_t::output_t::stderr);
   UNUSED(sink1);
   UNUSED(sink2);
}

TEST(stream_t, StringConstructor) {
    sink::stream_t sink1("stdout");
    sink::stream_t sink2("stderr");
    UNUSED(sink1);
    UNUSED(sink2);
}

TEST(stream_t, CanConsumeLogMessage) {
    sink::stream_t sink(sink::stream_t::output_t::stdout);
    sink.consume("test message for stream sink");
}
