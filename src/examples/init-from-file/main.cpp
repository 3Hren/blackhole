#include <fstream>
#include <iostream>

#include <blackhole/blackhole.hpp>
#include <blackhole/repository/config/parser/rapidjson.hpp>

using namespace blackhole;

enum class level {
    debug,
    info,
    warning,
    error
};

void init(const rapidjson::Document& root) {
    auto configs = repository::config::parser::adapter_t<
        rapidjson::Value,
        std::vector<log_config_t>
    >::parse(root);
    auto& repository = repository_t::instance();
    repository.add_configs(configs);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: blackhole-example-init-from-file PATH" << std::endl;
        return 1;
    }

    const std::string& path(argv[1]);
    std::ifstream stream(path);
    std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    rapidjson::Document doc;
    doc.Parse<0>(content.c_str());
    if (doc.HasParseError()) {
        std::cout << doc.GetParseError() << std::endl;
        return 1;
    }

    init(doc);
    verbose_logger_t<level> log = repository_t::instance().root<level>();

    BH_LOG(log, level::info,   "this is just testing message")(
        attribute::make("id", 42)
    );
    return 0;
}
