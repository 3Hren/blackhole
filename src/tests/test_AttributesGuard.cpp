#include <blackhole/formatter/string.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/sink/stream.hpp>

#include "global.hpp"

#include <boost/thread.hpp>

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

TEST(AttributesGuard, BasicUsage) {
    auto logger = logger_factory_t::create();

    {
        scoped_attributes_t guard1(logger, log::attributes_t({{"att1", 1}, {"att2", 2}}));

        auto record1 = logger.open_record();

        EXPECT_TRUE(record1.attributes.count("att1") > 0);
        EXPECT_TRUE(record1.attributes["att1"].value == log::attribute_value_t(1));

        EXPECT_TRUE(record1.attributes.count("att2") > 0);
        EXPECT_TRUE(record1.attributes["att2"].value == log::attribute_value_t(2));

        {
            scoped_attributes_t guard2(logger, log::attributes_t({{"att1", 10}, {"att3", 3}}));

            auto record2 = logger.open_record();

            EXPECT_TRUE(record2.attributes.count("att1") > 0);
            EXPECT_TRUE(record2.attributes["att1"].value == log::attribute_value_t(10));

            EXPECT_TRUE(record2.attributes.count("att2") > 0);
            EXPECT_TRUE(record2.attributes["att2"].value == log::attribute_value_t(2));

            EXPECT_TRUE(record2.attributes.count("att3") > 0);
            EXPECT_TRUE(record2.attributes["att3"].value == log::attribute_value_t(3));
        }

        auto record3 = logger.open_record();

        EXPECT_TRUE(record3.attributes.count("att1") > 0);
        EXPECT_TRUE(record3.attributes["att1"].value == log::attribute_value_t(1));

        EXPECT_TRUE(record3.attributes.count("att2") > 0);
        EXPECT_TRUE(record3.attributes["att2"].value == log::attribute_value_t(2));

        EXPECT_EQ(0, record3.attributes.count("att3"));
    }

    auto record4 = logger.open_record();

    EXPECT_EQ(0, record4.attributes.count("att1"));
    EXPECT_EQ(0, record4.attributes.count("att2"));
    EXPECT_EQ(0, record4.attributes.count("att3"));
}

TEST(AttributesGuard, SwapLoggers) {
    auto logger1 = logger_factory_t::create();
    auto logger2 = logger_factory_t::create();

    scoped_attributes_t guard1(logger1, log::attributes_t({{"att1", 1}, {"att2", 2}}));
    scoped_attributes_t guard2(logger2, log::attributes_t({{"att3", 3}, {"att4", 4}}));

    {
        auto record1 = logger1.open_record();

        EXPECT_TRUE(record1.attributes.count("att1") > 0);
        EXPECT_TRUE(record1.attributes["att1"].value == log::attribute_value_t(1));

        EXPECT_TRUE(record1.attributes.count("att2") > 0);
        EXPECT_TRUE(record1.attributes["att2"].value == log::attribute_value_t(2));

        EXPECT_EQ(0, record1.attributes.count("att3"));
        EXPECT_EQ(0, record1.attributes.count("att4"));

        auto record2 = logger2.open_record();

        EXPECT_TRUE(record2.attributes.count("att3") > 0);
        EXPECT_TRUE(record2.attributes["att3"].value == log::attribute_value_t(3));

        EXPECT_TRUE(record2.attributes.count("att4") > 0);
        EXPECT_TRUE(record2.attributes["att4"].value == log::attribute_value_t(4));

        EXPECT_EQ(0, record2.attributes.count("att1"));
        EXPECT_EQ(0, record2.attributes.count("att2"));
    }

    swap(logger1, logger2);

    {
        auto record1 = logger2.open_record();

        EXPECT_TRUE(record1.attributes.count("att1") > 0);
        EXPECT_TRUE(record1.attributes["att1"].value == log::attribute_value_t(1));

        EXPECT_TRUE(record1.attributes.count("att2") > 0);
        EXPECT_TRUE(record1.attributes["att2"].value == log::attribute_value_t(2));

        EXPECT_EQ(0, record1.attributes.count("att3"));
        EXPECT_EQ(0, record1.attributes.count("att4"));

        auto record2 = logger1.open_record();

        EXPECT_TRUE(record2.attributes.count("att3") > 0);
        EXPECT_TRUE(record2.attributes["att3"].value == log::attribute_value_t(3));

        EXPECT_TRUE(record2.attributes.count("att4") > 0);
        EXPECT_TRUE(record2.attributes["att4"].value == log::attribute_value_t(4));

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
        scoped_attributes_t guard(*logger, log::attributes_t({{"attr", value}}));

        barrier->wait();

        auto record = logger->open_record();

        EXPECT_TRUE(record.attributes.count("attr") > 0);
        EXPECT_TRUE(record.attributes["attr"].value == log::attribute_value_t(value));

        barrier->wait();
    }
};

} // namespace

TEST(AttributesGuard, ThreadLocality) {
    auto logger = logger_factory_t::create();
    boost::barrier barrier(2);

    thread_tester_t tester1 {&barrier, &logger, 1};
    boost::thread t1(&thread_tester_t::run, &tester1);

    thread_tester_t tester2 {&barrier, &logger, 2};
    boost::thread t2(&thread_tester_t::run, &tester2);

    t1.join();
    t2.join();
}
