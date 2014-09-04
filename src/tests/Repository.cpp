#include <blackhole/repository.hpp>
#include <blackhole/sink/null.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(repository_t, CreateBaseLogger) {
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

TEST(repository_t, FrontendRegistration) {
    /*!
     * Check frontend registration process.
     * By default Blackhole doesn't contain any frontends other than
     * string/stream.
     * But after registration it should be able to create them.
     */
    repository_t repository;
    EXPECT_FALSE((repository.registered<sink::null_t, formatter::string_t>()));

    repository.registrate<sink::null_t, formatter::string_t>();
    EXPECT_TRUE((repository.registered<sink::null_t, formatter::string_t>()));
}
