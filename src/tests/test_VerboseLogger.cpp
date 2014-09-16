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
    log.verbosity(testing::info);

    verbose_logger_t<testing::level> other(std::move(log));

    EXPECT_EQ(testing::info, other.verbosity());
}

TEST(verbose_logger_t, MoveImplicitConstructor) {
    verbose_logger_t<testing::level> log;
    log.verbosity(testing::info);
    EXPECT_EQ(testing::info, log.verbosity());

    verbose_logger_t<testing::level> other = std::move(log);
    EXPECT_EQ(testing::info, other.verbosity());
}

TEST(verbose_logger_t, MoveAssignment) {
    verbose_logger_t<testing::level> log;
    log.verbosity(testing::info);
    EXPECT_EQ(testing::info, log.verbosity());

    verbose_logger_t<testing::level> other;
    EXPECT_EQ(testing::debug, other.verbosity());

    other = std::move(log);
    EXPECT_EQ(testing::info, other.verbosity());
}

TEST(verbose_logger_t, OpenRecordByDefault) {
    std::unique_ptr<mock::frontend_t> frontend;

    verbose_logger_t<testing::level> log;
    log.add_frontend(std::move(frontend));
    record_t record = log.open_record(testing::level::debug);
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
    log.open_record(blackhole::attribute::set_t({
        blackhole::attribute::make("key", 42)
    }));
}

TEST(verbose_logger_t, Manual) {
    verbose_logger_t<testing::level> log;

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
