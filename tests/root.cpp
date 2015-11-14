#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "blackhole/extensions/writer.hpp"
#include <blackhole/logger.hpp>
#include <blackhole/record.hpp>
#include <blackhole/root.hpp>
#include <blackhole/scoped.hpp>

#include "mocks/handler.hpp"

namespace blackhole {
namespace testing {

using ::testing::Invoke;
using ::testing::_;

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

    logger.log(0, "GET /porn.png HTTP/1.1 - {}/{}", pack, [](writer_t& writer) {
        writer.write("GET /porn.png HTTP/1.1 - {}/{}", 42, 2345);
    });
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
    const auto scoped = logger.scoped({{"key#1", {42}}});

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
    const auto s1 = logger.scoped({{"key#1", {42}}});

    // NOTE: The following log will contain flattened attributes list.
    {
        const auto s2 = logger.scoped({{"key#2", {100}}});
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
    const auto scoped = logger2.scoped({{"key#1", {42}}});

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
    const auto s1 = logger2.scoped({{"key#1", {42}}});

    {
        const auto s2 = logger2.scoped({{"key#2", {"value#2"}}});
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
    const auto s1 = logger2.scoped({{"key#1", {42}}});

    {
        const auto s2 = logger2.scoped({{"key#2", {"value#2"}}});
        {
            const auto s3 = logger2.scoped({{"key#3", {100}}});
            logger1 = std::move(logger2);
            logger1.log(0, "-");
        }
        logger1.log(0, "-");
    }
    logger1.log(0, "-");
}

}  // namespace testing
}  // namespace blackhole
