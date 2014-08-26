#include <blackhole/repository.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(Repository, CreateBaseLogger) {
    /*!
     * Check repository's default constructor.
     * Default trivial frontend must be registered with it. Also it's possible
     * to create that trivial logger and it'll be valid.
     */
    repository_t repository;

    EXPECT_TRUE((repository.registered<sink::stream_t, formatter::string_t>()));

    auto log = repository.create<logger_base_t>("root");
    auto record = log.open_record();
    EXPECT_TRUE(record);
}
