#include "Mocks.hpp"

namespace expr = blackhole::expression;

TEST(HasAttribute, ReturnsTrueIfDynamicAttributeExists) {
    auto filter = expr::has_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", {42}}};
    EXPECT_TRUE(filter(attributes));
}

TEST(HasAttribute, ReturnsFalseIfDynamicAttributeNotExists) {
    auto filter = expr::has_attr<std::int32_t>("non-existing-attribute");
    log::attributes_t attributes = {{"custom", {42}}};
    EXPECT_FALSE(filter(attributes));
}

TEST(HasAttribute, ReturnsFalseIfDynamicAttributeExistsButTypeMismatched) {
    auto filter = expr::has_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", {3.1415}}};
    EXPECT_FALSE(filter(attributes));
}

TEST(HasAttribute, ReturnsTrueIfBothDynamicAttributeNotExistsAndTypeMismatched) {
    auto filter = expr::has_attr<std::int32_t>("non-existing-attribute");
    log::attributes_t attributes = {{"custom", {3.1415}}};
    EXPECT_FALSE(filter(attributes));
}

TEST(GetAttribute, CanExtractDynamicAttribute) {
    auto filter = expr::get_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", {42}}};
    EXPECT_EQ(42, filter(attributes));
}

TEST(GetAttribute, ThrowsExceptionIfCannotExtractAttribute) {
    /*! It's okay that `get_attr` throws exceptions, cause there is no correct value to be
        returned if there is no value actually or its type not equal with expected type.
        You must check `has_attr<T>()` before extracting it.
        If not, an exception will be thrown and caught inside log core. */
    auto filter = expr::get_attr<std::int32_t>("non-existing-attribute");
    log::attributes_t attributes = {{"custom", {42}}};
    EXPECT_THROW(filter(attributes), std::logic_error);
}

TEST(GetAttribute, ThrowsExceptionIfAttributeHasTypedMismatch) {
    auto filter = expr::get_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", {3.1415}}};
    EXPECT_THROW(filter(attributes), boost::bad_get);
}

TEST(GetAttribute, CanCompareAttributeDeferredly) {
    auto filter = expr::get_attr<std::int32_t>("custom") == 42;
    log::attributes_t attributes = {{"custom", {42}}};
    EXPECT_TRUE(filter(attributes));
}

TEST(GetAttribute, FailsIfDeferredlyAttributeComparingFails) {
    auto filter = expr::get_attr<std::int32_t>("custom") == 666;
    log::attributes_t attributes = {{"custom", {42}}};
    EXPECT_FALSE(filter(attributes));
}

TEST(GetAttribute, CanExtractStaticAttribute) {
    auto filter = expr::get_attr(keyword::timestamp());
    log::attributes_t attributes = {{"timestamp", {std::time_t(100500)}}};
    EXPECT_EQ(std::time_t(100500), filter(attributes));
}

TEST(FilterAttribute, ComplexAttributeFiltering) {
    auto filter = expr::has_attr<std::int32_t>("custom") && expr::get_attr<std::int32_t>("custom") == 42;
    log::attributes_t attributes = {{"custom", {42}}};
    EXPECT_TRUE(filter(attributes));
}

//!@todo: Make || operations in filtering
//!@todo: Setting, checking and extracting enum variable.
