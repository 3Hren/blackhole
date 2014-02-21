#include <blackhole/sink/files/rotation.hpp>

#include "global.hpp"
#include "mocks/files.hpp"

using namespace blackhole;

TEST(size_t, Class) {
    sink::rotation::watcher::size_t watcher(1024);
    UNUSED(watcher);
}

TEST(size_t, InitializationThroughConfig) {
    sink::rotation::watcher::config_t<sink::rotation::watcher::size_t> config = { 1024 };
    sink::rotation::watcher::size_t watcher(config);
    UNUSED(watcher);
}

TEST(size_t, TriggerIfIncomingSizeGreaterOrEqualThanMaximum) {
    sink::rotation::watcher::size_t watcher(1024);

    NiceMock<mock::files::backend_t> backend;

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, size(std::string("test.log")))
            .WillOnce(Return(1024 - std::string("message").size()));

    EXPECT_TRUE(watcher(backend, "message"));
}

TEST(size_t, NotTriggerIfIncomingSizeLessThanMaximum) {
    sink::rotation::watcher::size_t watcher(1024);

    NiceMock<mock::files::backend_t> backend;

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, size(std::string("test.log")))
            .WillOnce(Return(1024 - std::string("message").size() - 1));

    EXPECT_FALSE(watcher(backend, "message"));
}
