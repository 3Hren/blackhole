#include "Mocks.hpp"

using namespace blackhole;

namespace testing {

enum level : std::uint64_t { debug, info, warn, error };

} // namespace testing

TEST(verbose_logger_t, Class) {
    verbose_logger_t<testing::level> log;
    UNUSED(log);
}

TEST(verbose_logger_t, OpenRecordByDefault) {
    std::unique_ptr<mock::frontend_t> frontend;

    verbose_logger_t<testing::level> log;
    log.add_frontend(std::move(frontend));
    log::record_t record = log.open_record(testing::level::debug);
    EXPECT_TRUE(record.valid());
}

TEST(verbose_logger_t, OpenRecordForValidVerbosityLevel) {
    std::unique_ptr<mock::frontend_t> frontend;

    verbose_logger_t<testing::level> log;
    log.add_frontend(std::move(frontend));
    log.set_filter(keyword::severity<testing::level>() >= testing::level::info);
    EXPECT_FALSE(log.open_record(testing::level::debug).valid());
    EXPECT_TRUE(log.open_record(testing::level::info).valid());
    EXPECT_TRUE(log.open_record(testing::level::warn).valid());
    EXPECT_TRUE(log.open_record(testing::level::error).valid());
}

TEST(verbose_logger_t, Manual) {
    verbose_logger_t<testing::level> log;

    //!@note: Factory starts here...
    auto formatter = std::make_unique<formatter::string_t>("[]: %(message)s [%(...L)s]");
    auto sink = std::make_unique<sink::file_t<>>(sink::file_t<>::config_type("/dev/stdout"));
    auto frontend = std::make_unique<frontend_t<formatter::string_t, sink::file_t<>>>(std::move(formatter), std::move(sink));
    //!@note ... till here.
    log.add_frontend(std::move(frontend));

    //!@note: Next lines can be hidden via macro: LOG(log, debug, "Message %s", "Hell")(keyword::answer = 42, keyword::blah = "WAT?", keyword::make("urgent", 1));
    log::record_t record = log.open_record(testing::level::error);
    if (record.valid()) {
        record.attributes.insert(keyword::message() = utils::format("Some message from: '%s'!", "Hell"));
        // Add another attributes.
        log.push(std::move(record));
    }
}
