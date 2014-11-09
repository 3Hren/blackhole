#include <blackhole/formatter/string.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/frontend/files.hpp>

#include "global.hpp"
#include "mocks/frontend.hpp"

using namespace blackhole;

TEST(verbose_logger_t, Manual) {
    verbose_logger_t<testing::level> log(testing::debug);

    // Factory starts here...
    auto formatter = aux::util::make_unique<
        formatter::string_t
    >("[]: %(message)s [%(...L)s]");

    auto sink = aux::util::make_unique<
        sink::files_t<>
    >(sink::files_t<>::config_type("/dev/stdout"));

    auto frontend = aux::util::make_unique<
        frontend_t<
            formatter::string_t,
            sink::files_t<>
        >
    >(std::move(formatter), std::move(sink));
    // ... till here.
    log.add_frontend(std::move(frontend));

    // Next lines can be hidden via macro:
    // LOG(log, debug, "Message %s", "Hell")(keyword::answer = 42, keyword::blah = "WAT?", keyword::make("urgent", 1));
    record_t record = log.open_record(testing::level::error);
    if (record.valid()) {
        record.insert(keyword::message() = utils::format("Some message from: '%s'!", "Hell"));
        // Add another attributes.
        log.push(std::move(record));
    }
}
