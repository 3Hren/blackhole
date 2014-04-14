#include <blackhole/formatter/string.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/frontend/files.hpp>

#include "global.hpp"
#include "mocks/frontend.hpp"

using namespace blackhole;

TEST(verbose_logger_t, Class) {
    verbose_logger_t<testing::level> log;
    UNUSED(log);
}

TEST(verbose_logger_t, MoveExplicitConstructor) {
    verbose_logger_t<testing::level> log;
    verbose_logger_t<testing::level> other(std::move(log));
    UNUSED(other);
}

TEST(verbose_logger_t, MoveImplicitConstructor) {
    verbose_logger_t<testing::level> log;
    verbose_logger_t<testing::level> other = std::move(log);
    UNUSED(other);
}

TEST(verbose_logger_t, MoveAssignment) {
    verbose_logger_t<testing::level> log;
    verbose_logger_t<testing::level> other;
    other = std::move(log);
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

TEST(verbose_logger_t, ImportsOpenRecordFromAncestor) {
    verbose_logger_t<testing::level> log;
    log.open_record(blackhole::log::attributes_t({
        blackhole::attribute::make("key", 42)
    }));
}

TEST(verbose_logger_t, Manual) {
    verbose_logger_t<testing::level> log;

    //!@note: Factory starts here...
    auto formatter = utils::make_unique<formatter::string_t>("[]: %(message)s [%(...L)s]");
    auto sink = utils::make_unique<sink::files_t<>>(sink::files_t<>::config_type("/dev/stdout"));
    auto frontend = utils::make_unique<frontend_t<formatter::string_t, sink::files_t<>>>(std::move(formatter), std::move(sink));
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
