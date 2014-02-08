#include "Mocks.hpp"
// test json is valid
// test parse formatter config into formatter_config_t
// test parse sink config -//-
// test parse entire config including log name.
// test parse multiple frontends
// opt: test throw exception when frontend type not registered
// opt: test -//-                 sink -//-

class parser_t {
public:
    static std::vector<log_config_t> parse(const rapidjson::Value& root) {
        std::vector<log_config_t> configs;
        for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {
            log_config_t config;
            config.name = it->name.GetString();

            // Every frontend.
            for (auto it2 = it->value.Begin(); it2 != it->value.End(); ++it2) {
                const rapidjson::Value& frontend_node = *it2;

                if (!(it2->HasMember("formatter") && it2->HasMember("sink"))) {
                    throw blackhole::error_t("absent 'formatter' or 'sink' section");
                }

                // Extract formatter and sink.
                const rapidjson::Value& formatter_node = frontend_node["formatter"];
                formatter_config_t formatter_config(formatter_node["type"].GetString());
                fill(formatter_config, formatter_node);

                const rapidjson::Value& sink_node = frontend_node["sink"];
                sink_config_t sink_config(sink_node["type"].GetString());
                fill(sink_config, sink_node);

                frontend_config_t frontend_config = { formatter_config, sink_config };
                config.frontends.push_back(frontend_config);
            }
            configs.push_back(config);
        }

        return configs;
    }

    template<typename T>
    static void fill(T& builder, const rapidjson::Value& node) {
        for (auto it = node.MemberBegin(); it != node.MemberEnd(); ++it) {
            std::string n(it->name.GetString());
            const rapidjson::Value& v = it->value;

            if (v.IsObject()) {
                auto b = builder[n];
                fill(b, v);
            } else {
                if (n != "type") {
                    if (v.IsBool()) {
                        builder[n] = v.GetBool();
                    } else if (v.IsInt()) {
                        builder[n] = v.GetInt();
                    } else if (v.IsInt64()) {
                        builder[n] = v.GetInt64();
                    } else if (v.IsUint()) {
                        builder[n] = v.GetUint();
                    } else if (v.IsUint64()) {
                        builder[n] = v.GetUint64();
                    } else if (v.IsDouble()) {
                        builder[n] = v.GetDouble();
                    } else if (v.IsString()) {
                        builder[n] = std::string(v.GetString());
                    }
                }
            }
        }
    }
};

class parser_test_case_t : public Test {
protected:
    rapidjson::Document doc;
public:
    parser_test_case_t() {}

protected:
    void SetUp() {
        std::ifstream stream("config/valid.json");
        std::string valid((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        doc.Parse<0>(valid.c_str());
    }
};

TEST_F(parser_test_case_t, NameParsing) {
    const std::vector<log_config_t>& configs = parser_t::parse(doc);
    ASSERT_EQ(1, configs.size());
    EXPECT_EQ("root", configs.at(0).name);
}

TEST_F(parser_test_case_t, CheckFormatterConfigAfterParsing) {
    const std::vector<log_config_t>& configs = parser_t::parse(doc);

    ASSERT_EQ(1, configs.size());
    ASSERT_EQ(1, configs.at(0).frontends.size());

    const formatter_config_t& fmt = configs.at(0).frontends.at(0).formatter;
    EXPECT_EQ("string", fmt.type);
    EXPECT_EQ("[%(level)s]: %(message)s", fmt["pattern"].to<std::string>());
}

TEST_F(parser_test_case_t, CheckSinkConfigAfterParsing) {
    const std::vector<log_config_t>& configs = parser_t::parse(doc);

    ASSERT_EQ(1, configs.size());
    ASSERT_EQ(1, configs.at(0).frontends.size());

    const sink_config_t& sink = configs.at(0).frontends.at(0).sink;
    EXPECT_EQ("files", sink.type);
    EXPECT_EQ("test.log", sink["path"].to<std::string>());
    EXPECT_EQ("test.log.%N", sink["rotation"]["pattern"].to<std::string>());
    EXPECT_EQ(5, sink["rotation"]["backups"].to<int>());
    EXPECT_EQ(1000000, sink["rotation"]["size"].to<int>()); //!@todo: We have a problem here.
}

TEST(parser_t, ThrowsExceptionIfFormatterSectionIsAbsent) {
    std::ifstream stream("config/no-valid-absent-formatter.json");
    std::string nonvalid((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    rapidjson::Document doc;
    doc.Parse<0>(nonvalid.c_str());
    EXPECT_THROW(parser_t::parse(doc), blackhole::error_t);
}

TEST(parser_t, ThrowsExceptionIfSinkSectionIsAbsent) {
    std::ifstream stream("config/no-valid-absent-sink.json");
    std::string nonvalid((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    rapidjson::Document doc;
    doc.Parse<0>(nonvalid.c_str());
    EXPECT_THROW(parser_t::parse(doc), blackhole::error_t);
}
