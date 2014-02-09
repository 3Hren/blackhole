#include <blackhole/log.hpp>
#include <blackhole/repository.hpp>
#include <blackhole/repository/config/parser/rapidjson.hpp>

using namespace blackhole;

enum class level {
    debug,
    info,
    warning,
    error
};

void init(const rapidjson::Document& root) {
    const std::vector<log_config_t>& configs = repository::config::parser_t<std::vector<log_config_t>>::parse(root);

    repository_t<level>& repository = repository_t<level>::instance();
    for (auto it = configs.begin(); it != configs.end(); ++it) {
        repository.init(*it);
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: blackhole-example-init-from-file PATH" << std::endl;
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
    verbose_logger_t<level> log = repository_t<level>::instance().root();

    BH_LOG(log, level::info,   "this is just testing message")(
        attribute::make("id", 42)
    );
    return 0;
}
