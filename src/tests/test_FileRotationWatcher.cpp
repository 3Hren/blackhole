#include "Mocks.hpp"

TEST(size_t, Class) {
    sink::watcher::size_t watcher(1024);
    UNUSED(watcher);
}

TEST(size_t, TriggerIfIncomingSizeGreaterOrEqualThanMaximum) {
    sink::watcher::size_t watcher(1024);

    NiceMock<mock::files::backend_t> backend;

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, size(std::string("test.log")))
            .WillOnce(Return(1024 - std::string("message").size()));

    EXPECT_TRUE(watcher(backend, "message"));
}

TEST(size_t, NotTriggerIfIncomingSizeLessThanMaximum) {
    sink::watcher::size_t watcher(1024);

    NiceMock<mock::files::backend_t> backend;

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, size(std::string("test.log")))
            .WillOnce(Return(1024 - std::string("message").size() - 1));

    EXPECT_FALSE(watcher(backend, "message"));
}
