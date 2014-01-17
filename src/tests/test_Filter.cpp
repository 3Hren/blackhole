#include "Mocks.hpp"

namespace expr = blackhole::expression;

TEST(HasAttribute, ReturnsTrueIfDynamicAttributeExists) {
    auto filter = expr::has_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_TRUE(filter(attributes));
}

TEST(HasAttribute, ReturnsFalseIfDynamicAttributeNotExists) {
    auto filter = expr::has_attr<std::int32_t>("non-existing-attribute");
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_FALSE(filter(attributes));
}

TEST(HasAttribute, ReturnsFalseIfDynamicAttributeExistsButTypeMismatched) {
    auto filter = expr::has_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", log::attribute_t(3.1415)}};
    EXPECT_FALSE(filter(attributes));
}

TEST(HasAttribute, ReturnsTrueIfBothDynamicAttributeNotExistsAndTypeMismatched) {
    auto filter = expr::has_attr<std::int32_t>("non-existing-attribute");
    log::attributes_t attributes = {{"custom", log::attribute_t(3.1415)}};
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterCustomAttribute, CanExtractDynamicAttribute) {
    auto filter = expr::get_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_EQ(42, filter(attributes));
}

TEST(FilterCustomAttribute, ThrowsExceptionIfNameMismatch) {
    /*! It's okay that `get_attr` throws exceptions, cause there is no correct value to be
        returned if there is no value actually or its type not equal with expected type.
        You must check `has_attr<T>()` before extracting it.
        If not, an exception will be thrown and caught inside log core. */
    auto filter = expr::get_attr<std::int32_t>("non-existing-attribute");
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_THROW(filter(attributes), std::logic_error);
}

TEST(FilterCustomAttribute, ThrowsExceptionIfTypeMismatch) {
    auto filter = expr::get_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", log::attribute_t(3.1415)}};
    EXPECT_THROW(filter(attributes), boost::bad_get);
}

TEST(FilterCustomAttribute, Get) {
    auto filter = expr::get_attr<std::int32_t>("custom") == 42;
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterCustomAttribute, FailsIfDeferredlyAttributeComparingFails) {
    auto filter = expr::get_attr<std::int32_t>("custom") == 666;
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterCustomAttribute, HasAndGetEq) {
    auto filter = expr::has_attr<std::int32_t>("custom") && expr::get_attr<std::int32_t>("custom") == 42;
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterCustomAttribute, HasAndGetLess) {
    auto filter = expr::has_attr<std::int32_t>("custom")
            && expr::get_attr<std::int32_t>("custom") < 42;
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_FALSE(filter(attributes));

    attributes = {{"custom", log::attribute_t(41)}};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterCustomAttribute, HasAndGetLessEq) {
    auto filter = expr::has_attr<std::int32_t>("custom")
            && expr::get_attr<std::int32_t>("custom") <= 42;
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_TRUE(filter(attributes));

    attributes = {{"custom", log::attribute_t(41)}};
    EXPECT_TRUE(filter(attributes));

    attributes = {{"custom", log::attribute_t(43)}};
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterKeywordAttribute, Get) {
    auto filter = expr::get_attr(keyword::timestamp()) == std::time_t(100500);
    log::attributes_t attributes = {{"timestamp", log::attribute_t(std::time_t(100500))}};
    EXPECT_TRUE(filter(attributes));
}

namespace testing {

enum weak_enum {
    low,
    high
};

} // namespace testing

TEST(FilterWeaklyTypedEnumAttribute, Has) {
    auto filter = expr::has_attr<testing::weak_enum>("weak_enum");
    log::attributes_t attributes = {{ "weak_enum", log::attribute_t(testing::high) }};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterWeaklyTypedEnumAttribute, Get) {
    auto filter = expr::get_attr<testing::weak_enum>("weak_enum") == testing::high;
    log::attributes_t attributes = {{ "weak_enum", log::attribute_t(testing::high) }};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterWeaklyTypedEnumAttribute, HasAndGetLess) {
    auto filter = expr::has_attr<testing::weak_enum>("weak_enum")
            && expr::get_attr<testing::weak_enum>("weak_enum") < testing::high;
    log::attributes_t attributes = {{ "weak_enum", log::attribute_t(testing::high) }};
    EXPECT_FALSE(filter(attributes));

    attributes = {{ "weak_enum", log::attribute_t(testing::low) }};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterWeaklyTypedEnumAttribute, HasAndGetLessEq) {
    auto filter = expr::has_attr<testing::weak_enum>("weak_enum")
            && expr::get_attr<testing::weak_enum>("weak_enum") <= testing::high;
    log::attributes_t attributes = {{ "weak_enum", log::attribute_t(testing::high) }};
    EXPECT_TRUE(filter(attributes));

    attributes = {{ "weak_enum", log::attribute_t(testing::low) }};
    EXPECT_TRUE(filter(attributes));
}

namespace testing {

enum class strong_enum {
    low,
    high
};

} // namespace testing

TEST(FilterStronglyTypedEnumAttribute, Has) {
    auto filter = expr::has_attr<testing::strong_enum>("strong_enum");
    log::attributes_t attributes = {{ "strong_enum", log::attribute_t(testing::strong_enum::high) }};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterStronglyTypedEnumAttribute, Get) {
    auto filter = expr::get_attr<testing::strong_enum>("strong_enum") == testing::strong_enum::high;
    log::attributes_t attributes = {{ "strong_enum", log::attribute_t(testing::strong_enum::high) }};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterStronglyTypedEnumAttribute, HasAndGetLess) {
    auto filter = expr::has_attr<testing::strong_enum>("strong_enum")
            && expr::get_attr<testing::strong_enum>("strong_enum") < testing::strong_enum::high;
    log::attributes_t attributes = {{ "strong_enum", log::attribute_t(testing::strong_enum::high) }};
    EXPECT_FALSE(filter(attributes));

    attributes = {{ "strong_enum", log::attribute_t(testing::strong_enum::low) }};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterStronglyTypedEnumAttribute, HasAndGetLessEq) {
    auto filter = expr::has_attr<testing::strong_enum>("strong_enum")
            && expr::get_attr<testing::strong_enum>("strong_enum") <= testing::strong_enum::high;
    log::attributes_t attributes = {{ "strong_enum", log::attribute_t(testing::strong_enum::high) }};
    EXPECT_TRUE(filter(attributes));

    attributes = {{ "strong_enum", log::attribute_t(testing::strong_enum::low) }};
    EXPECT_TRUE(filter(attributes));
}

namespace ts {

enum class severity {
    debug,
    info,
    warning,
    error
};

} // namespace ts

TEST(FilterSeverity, Has) {
    auto filter = expr::has_attr(keyword::severity<ts::severity>());
    log::attributes_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterSeverity, GetEq) {
    auto filter = expr::get_attr(keyword::severity<ts::severity>()) == ts::severity::info;
    log::attributes_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterSeverity, HasAndGetLess) {
    auto filter = expr::has_attr(keyword::severity<ts::severity>())
            && expr::get_attr(keyword::severity<ts::severity>()) < ts::severity::info;
    log::attributes_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    EXPECT_FALSE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::debug };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterSeverity, HasAndGetLessEq) {
    auto filter = expr::has_attr(keyword::severity<ts::severity>())
            && expr::get_attr(keyword::severity<ts::severity>()) <= ts::severity::info;
    log::attributes_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    EXPECT_TRUE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::debug };
    EXPECT_TRUE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::warning };
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterSeverity, HasAndGetGt) {
    auto filter = expr::has_attr(keyword::severity<ts::severity>())
            && expr::get_attr(keyword::severity<ts::severity>()) > ts::severity::info;
    log::attributes_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    EXPECT_FALSE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::debug };
    EXPECT_FALSE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::warning };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterSeverity, HasAndGetGtEq) {
    auto filter = expr::has_attr(keyword::severity<ts::severity>())
            && expr::get_attr(keyword::severity<ts::severity>()) >= ts::severity::info;
    log::attributes_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    EXPECT_TRUE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::debug };
    EXPECT_FALSE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::warning };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterCustomAttribute, HasOrHas) {
    auto filter = expr::has_attr<std::int32_t>("custom-1") || expr::has_attr<std::int32_t>("custom-2");
    log::attributes_t attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100501)}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-0", log::attribute_t(42)},
        {"custom-1", log::attribute_t(100501)}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-0", log::attribute_t(41)},
        {"custom-3", log::attribute_t(100500)}
    };
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterCustomAttribute, GetEqAndGetEq) {
    auto filter = expr::get_attr<std::int32_t>("custom-1") == 42 &&
            expr::get_attr<std::int32_t>("custom-2") == 100500;
    log::attributes_t attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100500)}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100500)}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100501)}
    };
    EXPECT_FALSE(filter(attributes));
}
