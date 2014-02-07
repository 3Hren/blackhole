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
//        if (root.Size() != 1) {
//            throw blackhole::error_t("");
//        }

        std::vector<log_config_t> configs;
        for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {
            log_config_t config;
            config.name = it->name.GetString();

            configs.push_back(config);
        }

        return configs;
    }
};

template<typename T>
T to(const boost::any& source) {
    T value;
    try {
        aux::any_to(source, value);
    } catch (boost::bad_any_cast&) {
        throw blackhole::error_t("conversion error");
    }
    return value;
}

std::string valid_cfg = R"(
{
    "root": [
        {
            "formatter": {
                "type": "string",
                "pattern": "[%(level)s]: %(message)s"
            },
            "sink": {
                "type": "files",
                "path": "test.log",
                "rotation": {
                    "pattern": "test.log.%N",
                    "backups": 5,
                    "size": 1000000
                }
            }
        }
    ]
}
)";

TEST(parser_t, NameParsing) {
    rapidjson::Document doc;
    doc.Parse<0>(valid_cfg.c_str());
    std::vector<log_config_t> configs = parser_t::parse(doc);

    ASSERT_EQ(1, configs.size());
    EXPECT_EQ("root", configs.at(0).name);
}
