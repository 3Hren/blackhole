#include <blackhole/repository.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(Repository, CreateBaseLogger) {
    //!@todo: Write description and expectations.
    repository_t repository;
    repository.create<logger_base_t>("root");
}
