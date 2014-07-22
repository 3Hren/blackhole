#include <boost/algorithm/string.hpp>

#include <blackhole/repository/config/parser/rapidjson.hpp>

#include "global.hpp"

using namespace blackhole;

namespace testing {

namespace config {

namespace valid {

static const std::string JSON = "{\
    'root': [\
        {\
            'formatter': {\
                'type': 'string',\
                'pattern': '[%(level)s]: %(message)s'\
            },\
            'sink': {\
                'type': 'files',\
                'path': 'test.log',\
                'rotation': {\
                    'pattern': 'test.log.%N',\
                    'backups': 5,\
                    'size': 1000000\
                }\
            }\
        }\
    ]\
}";

static const std::string MULTIPLE_FRONTENDS = "{\
    'root': [\
        {\
            'formatter': {\
                'type': 'string',\
                'pattern': '[%(level)s]: %(message)s'\
            },\
            'sink': {\
                'type': 'files',\
                'path': 'test.log'\
            }\
        },\
        {\
            'formatter': {\
                'type': 'json'\
            },\
            'sink': {\
                'type': 'tcp',\
                'host': 'localhost',\
                'port': 50030\
            }\
        }\
    ]\
}";

} // namespace valid

namespace invalid {

static const std::string ABSENT_FORMATTER = "{\
    'root': [\
        {\
            'sink': {\
                'type': 'files',\
                'path': 'test.log',\
                'rotation': {\
                    'pattern': 'test.log.%N',\
                    'backups': 5,\
                    'size': 1000000\
                }\
            }\
        }\
    ]\
}";

static const std::string ABSENT_SINK = "{\
    'root': [\
        {\
            'formatter': {\
                'type': 'string',\
                'pattern': '[%(level)s]: %(message)s'\
            }\
        }\
    ]\
}";

} // namespace invalid

} // namespace config

} // namespace testing

class parser_test_case_t : public Test {
protected:
    std::vector<log_config_t> configs;

    void SetUp() {
        std::string valid(boost::algorithm::replace_all_copy(config::valid::JSON, "'", "\""));
        rapidjson::Document doc;
        doc.Parse<0>(valid.c_str());
        ASSERT_FALSE(doc.HasParseError());
        configs = repository::config::parser::adapter_t<
            rapidjson::Value,
            std::vector<log_config_t>
        >::parse(doc);
    }
};

TEST_F(parser_test_case_t, NameParsing) {
    ASSERT_EQ(1, configs.size());
    EXPECT_EQ("root", configs.at(0).name);
}

TEST_F(parser_test_case_t, CheckFormatterConfigAfterParsing) {
    ASSERT_EQ(1, configs.size());
    ASSERT_EQ(1, configs.at(0).frontends.size());

    const formatter_config_t& fmt = configs.at(0).frontends.at(0).formatter;
    EXPECT_EQ("string", fmt.type());
    EXPECT_EQ("[%(level)s]: %(message)s", fmt["pattern"].to<std::string>());
}

TEST_F(parser_test_case_t, CheckSinkConfigAfterParsing) {
    ASSERT_EQ(1, configs.size());
    ASSERT_EQ(1, configs.at(0).frontends.size());

    const sink_config_t& sink = configs.at(0).frontends.at(0).sink;
    EXPECT_EQ("files", sink.type());
    EXPECT_EQ("test.log", sink["path"].to<std::string>());
    EXPECT_EQ("test.log.%N", sink["rotation"]["pattern"].to<std::string>());
    EXPECT_EQ(5, sink["rotation"]["backups"].to<int>());
    EXPECT_EQ(1000000, sink["rotation"]["size"].to<int>());
}

TEST(parser_t, ThrowsExceptionIfFormatterSectionIsAbsent) {
    std::string invalid(boost::algorithm::replace_all_copy(config::invalid::ABSENT_FORMATTER, "'", "\""));
    rapidjson::Document doc;
    doc.Parse<0>(invalid.c_str());
    ASSERT_FALSE(doc.HasParseError());
    typedef repository::config::parser::adapter_t<
        rapidjson::Value,
        std::vector<log_config_t>
    > parser_t;
    EXPECT_THROW(parser_t::parse(doc), blackhole::error_t);
}

TEST(parser_t, ThrowsExceptionIfSinkSectionIsAbsent) {
    std::string invalid(boost::algorithm::replace_all_copy(config::invalid::ABSENT_SINK, "'", "\""));
    rapidjson::Document doc;
    doc.Parse<0>(invalid.c_str());
    ASSERT_FALSE(doc.HasParseError());
    typedef repository::config::parser::adapter_t<
        rapidjson::Value,
        std::vector<log_config_t>
    > parser_t;
    EXPECT_THROW(parser_t::parse(doc), blackhole::error_t);
}

TEST(parser_t, MultipleFrontends) {
    std::string valid(boost::algorithm::replace_all_copy(config::valid::MULTIPLE_FRONTENDS, "'", "\""));
    rapidjson::Document doc;
    doc.Parse<0>(valid.c_str());
    ASSERT_FALSE(doc.HasParseError());

    std::vector<log_config_t> configs = repository::config::parser::adapter_t<
        rapidjson::Value,
        std::vector<log_config_t>
    >::parse(doc);
    ASSERT_EQ(1, configs.size());
    EXPECT_EQ("root", configs.at(0).name);
    ASSERT_EQ(2, configs.at(0).frontends.size());

    const frontend_config_t& front1 = configs.at(0).frontends.at(0);
    ASSERT_EQ("string", front1.formatter.type());
    ASSERT_EQ("files", front1.sink.type());

    const frontend_config_t& front2 = configs.at(0).frontends.at(1);
    ASSERT_EQ("json", front2.formatter.type());
    ASSERT_EQ("tcp", front2.sink.type());

}

namespace testing {

namespace config {

namespace invalid {

static const std::string ABSENT_FORMATTER_TYPE = "{\
    'root': [\
        {\
            'formatter': {\
                'pattern': '[%(level)s]: %(message)s'\
            },\
            'sink': {\
                'type': 'files',\
                'path': 'test.log',\
                'rotation': {\
                    'pattern': 'test.log.%N',\
                    'backups': 5,\
                    'size': 1000000\
                }\
            }\
        }\
    ]\
}";

} // namespace invalid

} // namespace config

} // namespace testing

TEST(parser_t, ThrowsExceptionIfFormatterTypeIsAbsent) {
    std::string invalid(boost::algorithm::replace_all_copy(config::invalid::ABSENT_FORMATTER_TYPE, "'", "\""));
    rapidjson::Document doc;
    doc.Parse<0>(invalid.c_str());
    ASSERT_FALSE(doc.HasParseError());
    typedef repository::config::parser::adapter_t<
        rapidjson::Value,
        std::vector<log_config_t>
    > parser_t;
    EXPECT_THROW(parser_t::parse(doc), blackhole::error_t);
}

namespace testing {

namespace config {

namespace valid {

static const std::string WITH_ARRAY = "{\
    'root': [\
        {\
            'formatter': {\
                'type': 'json',\
                'newline': true,\
                'mapping': {\
                    'message': '@message',\
                    'timestamp': '@timestamp'\
                },\
                'routing': { \
                    '/': ['@message', '@timestamp'],\
                    'fields': '*'\
                }\
            },\
            'sink': {\
                'type': 'stream',\
                'output': 'stdout'\
            }\
        }\
    ]\
}";

} // namespace valid

} // namespace config

} // namespace testing

TEST(parser_t, FormatterWithArray) {
    std::string valid(boost::algorithm::replace_all_copy(config::valid::WITH_ARRAY, "'", "\""));
    rapidjson::Document doc;
    doc.Parse<0>(valid.c_str());
    ASSERT_FALSE(doc.HasParseError());

    std::vector<log_config_t> configs = repository::config::parser::adapter_t<
        rapidjson::Value,
        std::vector<log_config_t>
    >::parse(doc);
    ASSERT_EQ(1, configs.size());
    EXPECT_EQ("root", configs.at(0).name);
    ASSERT_EQ(1, configs.at(0).frontends.size());

    const frontend_config_t& front = configs.at(0).frontends.at(0);
    ASSERT_EQ("json", front.formatter.type());
    ASSERT_EQ("stream", front.sink.type());

}
