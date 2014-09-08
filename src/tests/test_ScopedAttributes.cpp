#include <boost/thread.hpp>

#include <blackhole/formatter/string.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/scoped_attributes.hpp>
#include <blackhole/sink/stream.hpp>

#include "global.hpp"

#define TEST_SCOPED_ATTRIBUTES(Suite, Test) \
    TEST(scoped_attributes_t, Suite##_##Test)

using namespace blackhole;

namespace {

class logger_factory_t {
public:
    static logger_base_t create() {
        logger_base_t logger;
        auto formatter = aux::util::make_unique<
            formatter::string_t
        >("[%(timestamp)s]: %(message)s");

        auto sink = aux::util::make_unique<
            sink::stream_t
        >(sink::stream_t::output_t::stdout);

        auto frontend = aux::util::make_unique<
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
            attribute::set_t({
                attribute::make("att1", 1),
                attribute::make("att2", 2)
            })
        );

        auto record1 = logger.open_record();

        EXPECT_TRUE(record1.attributes().find("att1"));
        EXPECT_EQ(1, record1.extract<int>("att1"));

        EXPECT_TRUE(record1.attributes().find("att2"));
        EXPECT_EQ(2, record1.extract<int>("att2"));

        {
            scoped_attributes_t guard2(
                logger,
                attribute::set_t({
                    attribute::make("att1", 10),
                    attribute::make("att3", 3)
                })
            );

            const auto record2 = logger.open_record();

            EXPECT_TRUE(record2.attributes().find("att1"));
            EXPECT_EQ(10, record2.extract<int>("att1"));

            EXPECT_TRUE(record2.attributes().find("att2"));
            EXPECT_EQ(2, record2.extract<int>("att2"));

            EXPECT_TRUE(record2.attributes().find("att3"));
            EXPECT_EQ(3, record2.extract<int>("att3"));
        }

        auto record3 = logger.open_record();

        EXPECT_TRUE(record3.attributes().find("att1"));
        EXPECT_EQ(1, record3.extract<int>("att1"));

        EXPECT_TRUE(record3.attributes().find("att2"));
        EXPECT_EQ(2, record3.extract<int>("att2"));

        EXPECT_FALSE(record3.attributes().find("att3"));
    }

    auto record4 = logger.open_record();

    EXPECT_FALSE(record4.attributes().find("att1"));
    EXPECT_FALSE(record4.attributes().find("att2"));
    EXPECT_FALSE(record4.attributes().find("att3"));
}

TEST_SCOPED_ATTRIBUTES(Cons, ViaCopyingAttributes) {
    auto log = logger_factory_t::create();
    attribute::set_t attributes({
        attribute::make("att1", 1),
        attribute::make("att2", 2)
    });

    scoped_attributes_t guard(log, attributes);
    auto record = log.open_record();

    ASSERT_TRUE(record.attributes().find("att1"));
    EXPECT_EQ(1, record.extract<int>("att1"));

    ASSERT_TRUE(record.attributes().find("att2"));
    EXPECT_EQ(2, record.extract<int>("att2"));
}

TEST(ScopedAttributes, SwapLoggers) {
    auto logger1 = logger_factory_t::create();
    auto logger2 = logger_factory_t::create();

    scoped_attributes_t guard1(
        logger1,
        attribute::set_t({
            attribute::make("att1", 1),
            attribute::make("att2", 2)
        })
    );

    scoped_attributes_t guard2(
        logger2,
        attribute::set_t({
            attribute::make("att3", 3),
            attribute::make("att4", 4)
        })
    );

    {
        auto record1 = logger1.open_record();
        EXPECT_TRUE(record1.attributes().find("att1"));
        EXPECT_EQ(1, record1.extract<int>("att1"));

        EXPECT_TRUE(record1.attributes().find("att2"));
        EXPECT_EQ(2, record1.extract<int>("att2"));

        EXPECT_FALSE(record1.attributes().find("att3"));
        EXPECT_FALSE(record1.attributes().find("att4"));

        auto record2 = logger2.open_record();
        EXPECT_TRUE(record2.attributes().find("att3"));
        EXPECT_EQ(3, record2.extract<int>("att3"));

        EXPECT_TRUE(record2.attributes().find("att4"));
        EXPECT_EQ(4, record2.extract<int>("att4"));

        EXPECT_FALSE(record2.attributes().find("att1"));
        EXPECT_FALSE(record2.attributes().find("att2"));
    }

    swap(logger1, logger2);

    {
        auto record1 = logger2.open_record();
        EXPECT_TRUE(record1.attributes().find("att1"));
        EXPECT_EQ(1, record1.extract<int>("att1"));

        EXPECT_TRUE(record1.attributes().find("att2"));
        EXPECT_EQ(2, record1.extract<int>("att2"));

        EXPECT_FALSE(record1.attributes().find("att3"));
        EXPECT_FALSE(record1.attributes().find("att4"));

        auto record2 = logger1.open_record();
        EXPECT_TRUE(record2.attributes().find("att3"));
        EXPECT_EQ(3, record2.extract<int>("att3"));

        EXPECT_TRUE(record2.attributes().find("att4"));
        EXPECT_EQ(4, record2.extract<int>("att4"));

        EXPECT_FALSE(record2.attributes().find("att1"));
        EXPECT_FALSE(record2.attributes().find("att2"));
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
            attribute::set_t({
                attribute::make("attr", value)
            })
        );

        barrier->wait();

        auto record = logger->open_record();

        EXPECT_TRUE(record.attributes().find("attr"));
        EXPECT_EQ(value, record.extract<int>("attr"));

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

TEST(ScopedAttrbutes, CompatibleWithWrapperViaMovingAttributes) {
    auto log = logger_factory_t::create();
    wrapper_t<logger_base_t> wrapper(log, attribute::set_t({
        attribute::make("answer", 42)
    }));

    {
        scoped_attributes_t guard(
            wrapper,
            attribute::set_t({
                attribute::make("piece of", "shit")
            })
        );

        auto record = wrapper.open_record();

        ASSERT_TRUE(record.attributes().find("answer"));
        EXPECT_EQ(42, record.extract<int>("answer"));

        ASSERT_TRUE(record.attributes().find("piece of"));
        EXPECT_EQ("shit", record.extract<std::string>("piece of"));
    }

    auto record = wrapper.open_record();

    ASSERT_TRUE(record.attributes().find("answer"));
    EXPECT_EQ(42, record.extract<int>("answer"));

    EXPECT_FALSE(record.attributes().find("piece of"));
}

TEST(ScopedAttrbutes, CompatibleWithWrapperViaCopyingAttributes) {
    auto log = logger_factory_t::create();
    wrapper_t<logger_base_t> wrapper(log, attribute::set_t({
        attribute::make("answer", 42)
    }));

    {
        attribute::set_t attributes({
            attribute::make("piece of", "shit")
        });
        scoped_attributes_t guard(wrapper, attributes);

        auto record = wrapper.open_record();

        ASSERT_TRUE(record.attributes().find("answer"));
        EXPECT_EQ(42, record.extract<int>("answer"));

        ASSERT_TRUE(record.attributes().find("piece of"));
        EXPECT_EQ("shit", record.extract<std::string>("piece of"));
    }

    auto record = wrapper.open_record();

    ASSERT_TRUE(record.attributes().find("answer"));
    EXPECT_EQ(42, record.extract<int>("answer"));

    EXPECT_FALSE(record.attributes().find("piece of"));
}

TEST(ScopedAttributes, CorrectlyBehavesOnMovedLogger) {
    auto log = logger_base_t();

    // After moving, newly created log should explicitly set no deletion
    // function for thread local scoped attributes.
    auto other = std::move(log);

    scoped_attributes_t guard(other, attribute::set_t());
}
