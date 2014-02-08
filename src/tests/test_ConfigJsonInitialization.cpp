#include "Mocks.hpp"

#include "blackhole/repository/config/parser/rapidjson.hpp"

class parser_test_case_t : public Test {
protected:
    std::vector<log_config_t> configs;

    void SetUp() {
        std::ifstream stream("config/valid.json");
        std::string valid((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        rapidjson::Document doc;
        doc.Parse<0>(valid.c_str());
        ASSERT_FALSE(doc.HasParseError());
        configs = repository::config::parser_t<std::vector<log_config_t>>::parse(doc);
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
    EXPECT_EQ("string", fmt.type);
    EXPECT_EQ("[%(level)s]: %(message)s", fmt["pattern"].to<std::string>());
}

TEST_F(parser_test_case_t, CheckSinkConfigAfterParsing) {
    ASSERT_EQ(1, configs.size());
    ASSERT_EQ(1, configs.at(0).frontends.size());

    const sink_config_t& sink = configs.at(0).frontends.at(0).sink;
    EXPECT_EQ("files", sink.type);
    EXPECT_EQ("test.log", sink["path"].to<std::string>());
    EXPECT_EQ("test.log.%N", sink["rotation"]["pattern"].to<std::string>());
    EXPECT_EQ(5, sink["rotation"]["backups"].to<int>());
    EXPECT_EQ(1000000, sink["rotation"]["size"].to<int>());
}

TEST(parser_t, ThrowsExceptionIfFormatterSectionIsAbsent) {
    std::ifstream stream("config/no-valid-absent-formatter.json");
    std::string nonvalid((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    rapidjson::Document doc;
    doc.Parse<0>(nonvalid.c_str());
    ASSERT_FALSE(doc.HasParseError());
    EXPECT_THROW(repository::config::parser_t<std::vector<log_config_t>>::parse(doc), blackhole::error_t);
}

TEST(parser_t, ThrowsExceptionIfSinkSectionIsAbsent) {
    std::ifstream stream("config/no-valid-absent-sink.json");
    std::string nonvalid((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    rapidjson::Document doc;
    doc.Parse<0>(nonvalid.c_str());
    ASSERT_FALSE(doc.HasParseError());
    EXPECT_THROW(repository::config::parser_t<std::vector<log_config_t>>::parse(doc), blackhole::error_t);
}

TEST(parser_t, MultipleFrontends) {
    std::ifstream stream("config/valid-multiple-frontends.json");
    std::string valid((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    rapidjson::Document doc;
    doc.Parse<0>(valid.c_str());
    ASSERT_FALSE(doc.HasParseError());

    const std::vector<log_config_t>& configs = repository::config::parser_t<std::vector<log_config_t>>::parse(doc);
    ASSERT_EQ(1, configs.size());
    EXPECT_EQ("root", configs.at(0).name);
    ASSERT_EQ(2, configs.at(0).frontends.size());

    const frontend_config_t& front1 = configs.at(0).frontends.at(0);
    ASSERT_EQ("string", front1.formatter.type);
    ASSERT_EQ("files", front1.sink.type);

    const frontend_config_t& front2 = configs.at(0).frontends.at(1);
    ASSERT_EQ("json", front2.formatter.type);
    ASSERT_EQ("tcp", front2.sink.type);

}
