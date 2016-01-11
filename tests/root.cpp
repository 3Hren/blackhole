#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/record.hpp>
#include <blackhole/root.hpp>
#include <blackhole/scoped/keeper.hpp>

#include "mocks/handler.hpp"

namespace blackhole {
namespace testing {

using ::testing::Invoke;
using ::testing::Throw;
using ::testing::_;
using ::testing::internal::CaptureStdout;
using ::testing::internal::GetCapturedStdout;

TEST(RootLogger, Constructor) {
    root_logger_t logger({});
    logger.log(0, "GET /porn.png HTTP/1.1");
}

TEST(RootLogger, FilterConstructor) {
    int passed = 0;

    root_logger_t logger([&](const record_t&) -> bool {
        passed++;
        return false;
    }, {});

    logger.log(0, "GET /porn.png HTTP/1.1");

    EXPECT_EQ(1, passed);
}

TEST(RootLogger, MoveConstructorMovesFilter) {
    int passed = 0;

    root_logger_t original([&](const record_t&) -> bool {
        passed += 1;
        return true;
    }, {});

    root_logger_t logger(std::move(original));

    EXPECT_EQ(0, passed);
    logger.log(0, "-");

    EXPECT_EQ(1, passed);
}

TEST(RootLogger, MoveConstructorMovesHandlers) {
    typedef view_of<attributes_t>::type attribute_list;

    std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
    mock::handler_t* view = handler.get();

    std::vector<std::unique_ptr<handler_t>> handlers;
    handlers.push_back(std::move(handler));

    root_logger_t original(std::move(handlers));

    EXPECT_CALL(*view, execute(_))
        .Times(1);

    root_logger_t logger(std::move(original));

    logger.log(0, "GET /porn.png HTTP/1.1");
}

TEST(RootLogger, MoveConstructorMovesScopedAttributes) {
    typedef view_of<attributes_t>::type attribute_list;

    std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
    mock::handler_t* view = handler.get();

    std::vector<std::unique_ptr<handler_t>> handlers;
    handlers.push_back(std::move(handler));

    root_logger_t original(std::move(handlers));

    // Logger must outlive its scoped attributes, that's why we need such variable.
    std::unique_ptr<root_logger_t> logger;
    const scoped::keeper_t scoped(original, {{"key#1", {42}}});

    // All scoped attributes should be assigned to the new owner.
    logger.reset(new root_logger_t(std::move(original)));

    EXPECT_CALL(*view, execute(_))
        .Times(1)
        .WillOnce(Invoke([](const record_t& record) {
            EXPECT_EQ("GET /porn.png HTTP/1.1", record.message().to_string());
            EXPECT_EQ("GET /porn.png HTTP/1.1", record.formatted().to_string());
            EXPECT_EQ(0, record.severity());
            ASSERT_EQ(1, record.attributes().size());
            EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(0).get());
        }));

    logger->log(0, "GET /porn.png HTTP/1.1");
}

TEST(RootLogger, MoveConstructorMovesNestedScopedAttributes) {
    std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
    mock::handler_t* view = handler.get();

    std::vector<std::unique_ptr<handler_t>> handlers;
    handlers.push_back(std::move(handler));

    EXPECT_CALL(*view, execute(_))
        .Times(2)
        .WillOnce(Invoke([](const record_t& record) {
            ASSERT_EQ(2, record.attributes().size());
            EXPECT_EQ((attribute_list{{"key#2", {"value#2"}}}), record.attributes().at(0).get());
            EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(1).get());
        }))
        .WillOnce(Invoke([](const record_t& record) {
            ASSERT_EQ(1, record.attributes().size());
            EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(0).get());
        }));;

    std::unique_ptr<root_logger_t> logger;

    root_logger_t original(std::move(handlers));
    const scoped::keeper_t s1(original, {{"key#1", {42}}});

    {
        const scoped::keeper_t s2(original, {{"key#2", {"value#2"}}});
        // All scoped attributes should be assigned to the new owner.
        logger.reset(new root_logger_t(std::move(original)));
        logger->log(0, "-");
    }

    logger->log(0, "-");
}

TEST(RootLogger, LogInvokesDispatchingRecordToHandlers) {
    std::vector<std::unique_ptr<handler_t>> handlers;
    std::vector<mock::handler_t*> handlers_view;

    for (int i = 0; i < 4; ++i) {
        std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
        handlers_view.push_back(handler.get());
        handlers.push_back(std::move(handler));
    }

    for (auto handler : handlers_view) {
        EXPECT_CALL(*handler, execute(_))
            .Times(1)
            .WillOnce(Invoke([](const record_t& record) {
                EXPECT_EQ("GET /porn.png HTTP/1.1", record.message().to_string());
                EXPECT_EQ("GET /porn.png HTTP/1.1", record.formatted().to_string());
                EXPECT_EQ(0, record.severity());
                EXPECT_EQ(0, record.attributes().size());
            }));
    }

    root_logger_t logger(std::move(handlers));

    logger.log(0, "GET /porn.png HTTP/1.1");
}

TEST(RootLogger, LogWithAttributesInvokesDispatchingRecordToHandlers) {
    typedef view_of<attributes_t>::type attribute_list;

    std::vector<std::unique_ptr<handler_t>> handlers;
    std::vector<mock::handler_t*> handlers_view;

    for (int i = 0; i < 4; ++i) {
        std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
        handlers_view.push_back(handler.get());
        handlers.push_back(std::move(handler));
    }

    for (auto handler : handlers_view) {
        EXPECT_CALL(*handler, execute(_))
            .Times(1)
            .WillOnce(Invoke([&](const record_t& record) {
                EXPECT_EQ("GET /porn.png HTTP/1.1", record.message().to_string());
                EXPECT_EQ("GET /porn.png HTTP/1.1", record.formatted().to_string());
                EXPECT_EQ(0, record.severity());
                ASSERT_EQ(1, record.attributes().size());
                EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(0).get());
            }));
    }

    root_logger_t logger(std::move(handlers));
    attribute_list attributes{{"key#1", {42}}};
    attribute_pack pack{attributes};

    logger.log(0, "GET /porn.png HTTP/1.1", pack);
}

TEST(RootLogger, LogWithAttributesAndFormatterInvokesDispatchingRecordToHandlers) {
    typedef view_of<attributes_t>::type attribute_list;

    std::vector<std::unique_ptr<handler_t>> handlers;
    std::vector<mock::handler_t*> handlers_view;

    for (int i = 0; i < 4; ++i) {
        std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
        handlers_view.push_back(handler.get());
        handlers.push_back(std::move(handler));
    }

    for (auto handler : handlers_view) {
        EXPECT_CALL(*handler, execute(_))
            .Times(1)
            .WillOnce(Invoke([&](const record_t& record) {
                EXPECT_EQ("GET /porn.png HTTP/1.1 - {}/{}", record.message().to_string());
                EXPECT_EQ("GET /porn.png HTTP/1.1 - 42/2345", record.formatted().to_string());
                EXPECT_EQ(0, record.severity());
                ASSERT_EQ(1, record.attributes().size());
                EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(0).get());
            }));
    }

    root_logger_t logger(std::move(handlers));

    attribute_list attributes{{"key#1", {42}}};
    attribute_pack pack{attributes};

    lazy_message_t message{{"GET /porn.png HTTP/1.1 - {}/{}"}, []() -> string_view {
        return {"GET /porn.png HTTP/1.1 - 42/2345"};
    }};
    logger.log(0, message, pack);
}

TEST(RootLogger, AssignmentMovesFilter) {
    int passed = 0;

    root_logger_t logger1([&](const record_t&) -> bool {
        passed += 1;
        return true;
    }, {});

    root_logger_t logger2([&](const record_t&) -> bool {
        passed += 2;
        return true;
    }, {});

    logger1 = std::move(logger2);
    logger1.log(0, "-");

    EXPECT_EQ(2, passed);
}

TEST(RootLogger, AssignmentMovesHandlers) {
    typedef view_of<attributes_t>::type attribute_list;

    std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
    mock::handler_t* view = handler.get();

    std::vector<std::unique_ptr<handler_t>> handlers;
    handlers.push_back(std::move(handler));

    EXPECT_CALL(*view, execute(_))
        .Times(1);

    root_logger_t logger1({});
    root_logger_t logger2(std::move(handlers));

    logger1 = std::move(logger2);

    logger1.log(0, "GET /porn.png HTTP/1.1");
}

TEST(RootLogger, LogWithScopedAttributes) {
    typedef view_of<attributes_t>::type attribute_list;

    std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
    mock::handler_t* view = handler.get();

    std::vector<std::unique_ptr<handler_t>> handlers;
    handlers.push_back(std::move(handler));

    EXPECT_CALL(*view, execute(_))
        .Times(1)
        .WillOnce(Invoke([](const record_t& record) {
            ASSERT_EQ(1, record.attributes().size());
            EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(0).get());
        }));

    root_logger_t logger(std::move(handlers));
    const scoped::keeper_t scoped(logger, {{"key#1", {42}}});

    logger.log(0, "GET /porn.png HTTP/1.1");
}

TEST(RootLogger, LogWithNestedScopedAttributes) {
    typedef view_of<attributes_t>::type attribute_list;

    std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
    mock::handler_t* view = handler.get();

    std::vector<std::unique_ptr<handler_t>> handlers;
    handlers.push_back(std::move(handler));

    EXPECT_CALL(*view, execute(_))
        .Times(2)
        .WillOnce(Invoke([](const record_t& record) {
            ASSERT_EQ(2, record.attributes().size());
            EXPECT_EQ((attribute_list{{"key#2", {100}}}), record.attributes().at(0).get());
            EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(1).get());
        }))
        .WillOnce(Invoke([](const record_t& record) {
            ASSERT_EQ(1, record.attributes().size());
            EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(0).get());
        }));

    root_logger_t logger(std::move(handlers));
    const scoped::keeper_t s1(logger, {{"key#1", {42}}});

    // NOTE: The following log will contain flattened attributes list.
    {
        const scoped::keeper_t s2(logger, {{"key#2", {100}}});
        logger.log(0, "GET /porn.png HTTP/1.1");
    }

    logger.log(0, "GET /porn.png HTTP/1.1");
}

TEST(RootLogger, AssignmentMovesScopedAttributes) {
    typedef view_of<attributes_t>::type attribute_list;

    std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
    mock::handler_t* view = handler.get();

    std::vector<std::unique_ptr<handler_t>> handlers;
    handlers.push_back(std::move(handler));

    EXPECT_CALL(*view, execute(_))
        .Times(1)
        .WillOnce(Invoke([](const record_t& record) {
            EXPECT_EQ("GET /porn.png HTTP/1.1", record.message().to_string());
            EXPECT_EQ("GET /porn.png HTTP/1.1", record.formatted().to_string());
            EXPECT_EQ(0, record.severity());
            ASSERT_EQ(1, record.attributes().size());
            EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(0).get());
        }));

    root_logger_t logger1({});
    root_logger_t logger2(std::move(handlers));
    const scoped::keeper_t scoped(logger2, {{"key#1", {42}}});

    // All scoped attributes should be assigned to the new owner.
    logger1 = std::move(logger2);

    logger1.log(0, "GET /porn.png HTTP/1.1");
}

TEST(RootLogger, AssignmentMovesNestedScopedAttributes) {
    std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
    mock::handler_t* view = handler.get();

    std::vector<std::unique_ptr<handler_t>> handlers;
    handlers.push_back(std::move(handler));

    EXPECT_CALL(*view, execute(_))
        .Times(2)
        .WillOnce(Invoke([](const record_t& record) {
            ASSERT_EQ(2, record.attributes().size());
            EXPECT_EQ((attribute_list{{"key#2", {"value#2"}}}), record.attributes().at(0).get());
            EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(1).get());
        }))
        .WillOnce(Invoke([](const record_t& record) {
            ASSERT_EQ(1, record.attributes().size());
            EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(0).get());
        }));;

    root_logger_t logger1({});
    root_logger_t logger2(std::move(handlers));
    const scoped::keeper_t s1(logger2, {{"key#1", {42}}});

    {
        const scoped::keeper_t s2(logger2, {{"key#2", "value#2"}});
        // All scoped attributes should be assigned to the new owner.
        logger1 = std::move(logger2);
        logger1.log(0, "-");
    }

    logger1.log(0, "-");
}

TEST(RootLogger, AssignmentMovesNestedTripleScopedAttributes) {
    std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
    mock::handler_t* view = handler.get();

    std::vector<std::unique_ptr<handler_t>> handlers;
    handlers.push_back(std::move(handler));

    EXPECT_CALL(*view, execute(_))
        .Times(3)
        .WillOnce(Invoke([](const record_t& record) {
            ASSERT_EQ(3, record.attributes().size());
            EXPECT_EQ((attribute_list{{"key#3", {100}}}), record.attributes().at(0).get());
            EXPECT_EQ((attribute_list{{"key#2", {"value#2"}}}), record.attributes().at(1).get());
            EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(2).get());
        }))
        .WillOnce(Invoke([](const record_t& record) {
            ASSERT_EQ(2, record.attributes().size());
            EXPECT_EQ((attribute_list{{"key#2", {"value#2"}}}), record.attributes().at(0).get());
            EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(1).get());
        }))
        .WillOnce(Invoke([](const record_t& record) {
            ASSERT_EQ(1, record.attributes().size());
            EXPECT_EQ((attribute_list{{"key#1", {42}}}), record.attributes().at(0).get());
        }));;

    root_logger_t logger1({});
    root_logger_t logger2(std::move(handlers));
    const scoped::keeper_t s1(logger2, {{"key#1", {42}}});

    {
        const scoped::keeper_t s2(logger2, {{"key#2", {"value#2"}}});
        {
            const scoped::keeper_t s3(logger2, {{"key#3", {100}}});
            logger1 = std::move(logger2);
            logger1.log(0, "-");
        }
        logger1.log(0, "-");
    }
    logger1.log(0, "-");
}

TEST(RootLogger, IgnoresExceptionsFromHandlers) {
    auto handler = new mock::handler_t;
    std::vector<std::unique_ptr<handler_t>> handlers;
    handlers.emplace_back(handler);

    root_logger_t logger(std::move(handlers));

    EXPECT_CALL(*handler, execute(_))
        .Times(1)
        .WillOnce(Throw(std::runtime_error("...")));

    CaptureStdout();
    EXPECT_NO_THROW(logger.log(0, "GET /porn.png HTTP/1.1"));

    const std::string actual = GetCapturedStdout();
    EXPECT_EQ("logging core error occurred: ...\n", actual);
}

TEST(RootLogger, IgnoresMarginalExceptionsFromHandlers) {
    auto handler = new mock::handler_t;
    std::vector<std::unique_ptr<handler_t>> handlers;
    handlers.emplace_back(handler);

    root_logger_t logger(std::move(handlers));

    EXPECT_CALL(*handler, execute(_))
        .Times(1)
        .WillOnce(Throw(42));

    CaptureStdout();
    EXPECT_NO_THROW(logger.log(0, "GET /porn.png HTTP/1.1"));

    const std::string actual = GetCapturedStdout();
    EXPECT_EQ("logging core error occurred: unknown\n", actual);
}

}  // namespace testing
}  // namespace blackhole
