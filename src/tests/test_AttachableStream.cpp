#include <blackhole/detail/stream/stream.hpp>

#include "global.hpp"

using namespace blackhole::aux;

TEST(ostringstreambuf, Class) {
    ostringstreambuf buf;
    UNUSED(buf);
}

TEST(ostringstreambuf, StoreDataString) {
    std::string storage;
    ostringstreambuf streambuf(storage);
    std::ostream stream(&streambuf);
    stream << "Blah";
    stream.flush();
    EXPECT_EQ("Blah", storage);
}

TEST(ostringstreambuf, StoreDataStringLongerThanInitialSize) {
    std::string storage;
    ostringstreambuf streambuf(storage);
    std::ostream stream(&streambuf);
    stream << "Blahblahblahblah-blah!";
    stream.flush();
    EXPECT_EQ("Blahblahblahblah-blah!", storage);
}

TEST(ostringstreambuf, CanAttachString) {
    std::string storage;
    ostringstreambuf streambuf;
    std::ostream stream(&streambuf);
    streambuf.attach(storage);
    stream << "Blah";
    stream.flush();
    EXPECT_EQ("Blah", storage);
}
