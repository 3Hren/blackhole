#include <boost/thread.hpp>

#include <blackhole/formatter/string.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/scoped_attributes.hpp>
#include <blackhole/sink/stream.hpp>

#include "global.hpp"

using namespace blackhole;

namespace {

class logger_factory_t {
public:
    static logger_base_t create() {
        logger_base_t logger;
        auto formatter = utils::make_unique<
            formatter::string_t
        >("[%(timestamp)s]: %(message)s");

        auto sink = utils::make_unique<
            sink::stream_t
        >(sink::stream_t::output_t::stdout);

        auto frontend = utils::make_unique<
            frontend_t<
                formatter::string_t,
                sink::stream_t
            >
        >(std::move(formatter), std::move(sink));

        logger.add_frontend(std::move(frontend));
        return logger;
    }
};

} // namespace

TEST(ScopedAttributes, BasicUsage) {
    auto logger = logger_factory_t::create();

    {
        scoped_attributes_t guard1(
            logger,
            log::attributes_t({
                attribute::make("att1", 1),
                attribute::make("att2", 2)
            })
        );

        auto record1 = logger.open_record();

        EXPECT_TRUE(record1.attributes.count("att1") > 0);
        EXPECT_EQ(log::attribute_value_t(1), record1.attributes["att1"].value);

        EXPECT_TRUE(record1.attributes.count("att2") > 0);
        EXPECT_EQ(log::attribute_value_t(2), record1.attributes["att2"].value);

        {
            scoped_attributes_t guard2(
                logger,
                log::attributes_t({
                    attribute::make("att1", 10),
                    attribute::make("att3", 3)
                })
            );

            auto record2 = logger.open_record();

            EXPECT_TRUE(record2.attributes.count("att1") > 0);
            EXPECT_EQ(log::attribute_value_t(10), record2.attributes["att1"].value);

            EXPECT_TRUE(record2.attributes.count("att2") > 0);
            EXPECT_EQ(log::attribute_value_t(2), record2.attributes["att2"].value);

            EXPECT_TRUE(record2.attributes.count("att3") > 0);
            EXPECT_EQ(log::attribute_value_t(3), record2.attributes["att3"].value);
        }

        auto record3 = logger.open_record();

        EXPECT_TRUE(record3.attributes.count("att1") > 0);
        EXPECT_EQ(log::attribute_value_t(1), record3.attributes["att1"].value);

        EXPECT_TRUE(record3.attributes.count("att2") > 0);
        EXPECT_EQ(log::attribute_value_t(2), record3.attributes["att2"].value);

        EXPECT_EQ(0, record3.attributes.count("att3"));
    }

    auto record4 = logger.open_record();

    EXPECT_EQ(0, record4.attributes.count("att1"));
    EXPECT_EQ(0, record4.attributes.count("att2"));
    EXPECT_EQ(0, record4.attributes.count("att3"));
}

TEST(ScopedAttributes, SwapLoggers) {
    auto logger1 = logger_factory_t::create();
    auto logger2 = logger_factory_t::create();

    scoped_attributes_t guard1(
        logger1,
        log::attributes_t({
            attribute::make("att1", 1),
            attribute::make("att2", 2)
        })
    );

    scoped_attributes_t guard2(
        logger2,
        log::attributes_t({
            attribute::make("att3", 3),
            attribute::make("att4", 4)
        })
    );

    {
        auto record1 = logger1.open_record();
        EXPECT_TRUE(record1.attributes.count("att1") > 0);
        EXPECT_EQ(log::attribute_value_t(1), record1.attributes["att1"].value);

        EXPECT_TRUE(record1.attributes.count("att2") > 0);
        EXPECT_EQ(log::attribute_value_t(2), record1.attributes["att2"].value);

        EXPECT_EQ(0, record1.attributes.count("att3"));
        EXPECT_EQ(0, record1.attributes.count("att4"));

        auto record2 = logger2.open_record();
        EXPECT_TRUE(record2.attributes.count("att3") > 0);
        EXPECT_EQ(log::attribute_value_t(3), record2.attributes["att3"].value);

        EXPECT_TRUE(record2.attributes.count("att4") > 0);
        EXPECT_EQ(log::attribute_value_t(4), record2.attributes["att4"].value);

        EXPECT_EQ(0, record2.attributes.count("att1"));
        EXPECT_EQ(0, record2.attributes.count("att2"));
    }

    swap(logger1, logger2);

    {
        auto record1 = logger2.open_record();
        EXPECT_TRUE(record1.attributes.count("att1") > 0);
        EXPECT_EQ(log::attribute_value_t(1), record1.attributes["att1"].value);

        EXPECT_TRUE(record1.attributes.count("att2") > 0);
        EXPECT_EQ(log::attribute_value_t(2), record1.attributes["att2"].value);

        EXPECT_EQ(0, record1.attributes.count("att3"));
        EXPECT_EQ(0, record1.attributes.count("att4"));

        auto record2 = logger1.open_record();
        EXPECT_TRUE(record2.attributes.count("att3") > 0);
        EXPECT_EQ(log::attribute_value_t(3), record2.attributes["att3"].value);

        EXPECT_TRUE(record2.attributes.count("att4") > 0);
        EXPECT_EQ(log::attribute_value_t(4), record2.attributes["att4"].value);

        EXPECT_EQ(0, record2.attributes.count("att1"));
        EXPECT_EQ(0, record2.attributes.count("att2"));
    }
}

namespace {

struct thread_tester_t {
    boost::barrier *barrier;
    logger_base_t *logger;
    int value;

    void run() {
        scoped_attributes_t guard(
            *logger,
            log::attributes_t({
                attribute::make("attr", value)
            })
        );

        barrier->wait();

        auto record = logger->open_record();

        EXPECT_TRUE(record.attributes.count("attr") > 0);
        EXPECT_EQ(log::attribute_value_t(value), record.attributes["attr"].value);

        barrier->wait();
    }
};

} // namespace

TEST(ScopedAttributes, ThreadLocality) {
    auto logger = logger_factory_t::create();
    boost::barrier barrier(2);

    thread_tester_t tester1 {&barrier, &logger, 1};
    boost::thread t1(&thread_tester_t::run, &tester1);

    thread_tester_t tester2 {&barrier, &logger, 2};
    boost::thread t2(&thread_tester_t::run, &tester2);

    t1.join();
    t2.join();
}

#include <blackhole/logger/wrapper.hpp>

TEST(ScopedAttrbutes, CompatibleWithWrapper) {
    auto log = logger_factory_t::create();
    wrapper_t<logger_base_t> wrapper(log, log::attributes_t({
        attribute::make("answer", 42)
    }));

    {
        scoped_attributes_t guard(
            wrapper,
            log::attributes_t({
                attribute::make("piece of", "shit")
            })
        );

        auto record = wrapper.open_record();

        ASSERT_TRUE(record.attributes.count("answer") > 0);
        EXPECT_EQ(log::attribute_value_t(42), record.attributes["answer"].value);

        ASSERT_TRUE(record.attributes.count("piece of") > 0);
        EXPECT_EQ(log::attribute_value_t("shit"), record.attributes["piece of"].value);
    }

    auto record = wrapper.open_record();

    ASSERT_TRUE(record.attributes.count("answer") > 0);
    EXPECT_EQ(log::attribute_value_t(42), record.attributes["answer"].value);

    EXPECT_EQ(0, record.attributes.count("piece of"));
}

TEST(ScopedAttributes, CorrectlyBehavesOnMovedLogger) {
    auto log = logger_base_t();

    // After moving, newly created log should explicitly set no deletion
    // function for thread local scoped attributes.
    auto other = std::move(log);

    scoped_attributes_t guard(other, log::attributes_t());
}
