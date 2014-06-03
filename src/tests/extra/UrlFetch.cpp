#include <boost/asio.hpp>

#include <urdl/option_set.hpp>

#include "../global.hpp"

namespace urlfetch {

struct request_t {
    std::string url;
    urdl::option_set options;
    long timeout;
};

TEST(request_t, Class) {
    urlfetch::request_t request;
    UNUSED(request);
}
