#include <boost/filesystem.hpp>

#include <blackhole/sink/files/backend.hpp>

#include "global.hpp"

using namespace blackhole;

namespace testing {

class boost_backend_t : public testing::Test {
protected:
    void SetUp() {
        boost::filesystem::create_directory(".testing");
    }

    void TearDown() {
        boost::filesystem::remove_all(".testing");
    }
};

} // namespace testing

TEST_F(boost_backend_t, Class) {
    sink::files::boost_backend_t backend(".testing/test.log");
    UNUSED(backend);
}

TEST_F(boost_backend_t, Opening) {
    sink::files::boost_backend_t backend(".testing/test.log");
    EXPECT_FALSE(backend.opened());
    EXPECT_FALSE(backend.exists("test.log"));

    EXPECT_TRUE(backend.open());

    EXPECT_TRUE(backend.opened());
    EXPECT_TRUE(backend.exists("test.log"));
}

TEST_F(boost_backend_t, Listdir) {
    sink::files::boost_backend_t backend(".testing/test.log");
    EXPECT_TRUE(backend.listdir().empty());

    backend.open();

    auto list = backend.listdir();
    ASSERT_FALSE(list.empty());
    EXPECT_EQ("test.log", list.at(0));
}
