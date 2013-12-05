#include "Mocks.hpp"

TEST(Filter, PassesIfHasAttribute) {
    auto filter = expr::has_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", 42}};
    EXPECT_TRUE(filter(attributes));
}

TEST(Filter, FailsIfHasNotAttribute) {
    auto filter = expr::has_attr<std::int32_t>("non-existing-attribute");
    log::attributes_t attributes = {{"custom", 42}};
    EXPECT_FALSE(filter(attributes));
}

TEST(Filter, FailsIfHasAttributeButTypeMismatch) {
    auto filter = expr::has_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", 3.1415}};
    EXPECT_FALSE(filter(attributes));
}

TEST(Filter, FailsIfBothHasNotAttributeAndTypeMismatch) {
    auto filter = expr::has_attr<std::int32_t>("non-existing-attribute");
    log::attributes_t attributes = {{"custom", 3.1415}};
    EXPECT_FALSE(filter(attributes));
}
