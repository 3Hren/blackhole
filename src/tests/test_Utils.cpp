#include "Mocks.hpp"

TEST(FactoryUtils, AnyIs) {
    boost::any any = 42;
    EXPECT_FALSE(aux::is<std::string>(any));
    EXPECT_TRUE(aux::is<int>(any));
}
