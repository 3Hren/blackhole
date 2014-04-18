#include <stdexcept>

#include <blackhole/expression.hpp>
#include <blackhole/keyword/severity.hpp>
#include <blackhole/keyword/timestamp.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(HasAttribute, ReturnsTrueIfDynamicAttributeExists) {
    auto filter = expression::has_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_TRUE(filter(attributes));
}

TEST(HasAttribute, ReturnsFalseIfDynamicAttributeNotExists) {
    auto filter = expression::has_attr<std::int32_t>("non-existing-attribute");
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_FALSE(filter(attributes));
}

TEST(HasAttribute, ReturnsFalseIfDynamicAttributeExistsButTypeMismatched) {
    auto filter = expression::has_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", log::attribute_t(3.1415)}};
    EXPECT_FALSE(filter(attributes));
}

TEST(HasAttribute, ReturnsTrueIfBothDynamicAttributeNotExistsAndTypeMismatched) {
    auto filter = expression::has_attr<std::int32_t>("non-existing-attribute");
    log::attributes_t attributes = {{"custom", log::attribute_t(3.1415)}};
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterCustomAttribute, CanExtractDynamicAttribute) {
    auto filter = expression::get_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_EQ(42, filter(attributes));
}

TEST(FilterCustomAttribute, ThrowsExceptionIfNameMismatch) {
    /*! It's okay that `get_attr` throws exceptions, cause there is no correct value to be
        returned if there is no value actually or its type not equal with expected type.
        You must check `has_attr<T>()` before extracting it.
        If not, an exception will be thrown and caught inside log core. */
    auto filter = expression::get_attr<std::int32_t>("non-existing-attribute");
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_THROW(filter(attributes), std::logic_error);
}

TEST(FilterCustomAttribute, ThrowsExceptionIfTypeMismatch) {
    auto filter = expression::get_attr<std::int32_t>("custom");
    log::attributes_t attributes = {{"custom", log::attribute_t(3.1415)}};
    EXPECT_THROW(filter(attributes), boost::bad_get);
}

TEST(FilterCustomAttribute, Get) {
    auto filter = expression::get_attr<std::int32_t>("custom") == 42;
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterCustomAttribute, FailsIfDeferredlyAttributeComparingFails) {
    auto filter = expression::get_attr<std::int32_t>("custom") == 666;
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterCustomAttribute, HasAndGetEq) {
    auto filter = expression::has_attr<std::int32_t>("custom") && expression::get_attr<std::int32_t>("custom") == 42;
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterCustomAttribute, HasAndGetLess) {
    auto filter = expression::has_attr<std::int32_t>("custom")
            && expression::get_attr<std::int32_t>("custom") < 42;
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_FALSE(filter(attributes));

    attributes = {{"custom", log::attribute_t(41)}};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterCustomAttribute, HasAndGetLessEq) {
    auto filter = expression::has_attr<std::int32_t>("custom")
            && expression::get_attr<std::int32_t>("custom") <= 42;
    log::attributes_t attributes = {{"custom", log::attribute_t(42)}};
    EXPECT_TRUE(filter(attributes));

    attributes = {{"custom", log::attribute_t(41)}};
    EXPECT_TRUE(filter(attributes));

    attributes = {{"custom", log::attribute_t(43)}};
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterKeywordAttribute, Get) {
    auto filter = expression::get_attr(keyword::timestamp()) == timeval{ 100500, 0 };
    log::attributes_t attributes = {{"timestamp", log::attribute_t(timeval{ 100500, 0 })}};
    EXPECT_TRUE(filter(attributes));
}

namespace testing {

enum weak_enum {
    low,
    high
};

} // namespace testing

TEST(FilterWeaklyTypedEnumAttribute, Has) {
    auto filter = expression::has_attr<testing::weak_enum>("weak_enum");
    log::attributes_t attributes = {{ "weak_enum", log::attribute_t(testing::high) }};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterWeaklyTypedEnumAttribute, Get) {
    auto filter = expression::get_attr<testing::weak_enum>("weak_enum") == testing::high;
    log::attributes_t attributes = {{ "weak_enum", log::attribute_t(testing::high) }};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterWeaklyTypedEnumAttribute, HasAndGetLess) {
    auto filter = expression::has_attr<testing::weak_enum>("weak_enum")
            && expression::get_attr<testing::weak_enum>("weak_enum") < testing::high;
    log::attributes_t attributes = {{ "weak_enum", log::attribute_t(testing::high) }};
    EXPECT_FALSE(filter(attributes));

    attributes = {{ "weak_enum", log::attribute_t(testing::low) }};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterWeaklyTypedEnumAttribute, HasAndGetLessEq) {
    auto filter = expression::has_attr<testing::weak_enum>("weak_enum")
            && expression::get_attr<testing::weak_enum>("weak_enum") <= testing::high;
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
    auto filter = expression::has_attr<testing::strong_enum>("strong_enum");
    log::attributes_t attributes = {{ "strong_enum", log::attribute_t(testing::strong_enum::high) }};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterStronglyTypedEnumAttribute, Get) {
    auto filter = expression::get_attr<testing::strong_enum>("strong_enum") == testing::strong_enum::high;
    log::attributes_t attributes = {{ "strong_enum", log::attribute_t(testing::strong_enum::high) }};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterStronglyTypedEnumAttribute, HasAndGetLess) {
    auto filter = expression::has_attr<testing::strong_enum>("strong_enum")
            && expression::get_attr<testing::strong_enum>("strong_enum") < testing::strong_enum::high;
    log::attributes_t attributes = {{ "strong_enum", log::attribute_t(testing::strong_enum::high) }};
    EXPECT_FALSE(filter(attributes));

    attributes = {{ "strong_enum", log::attribute_t(testing::strong_enum::low) }};
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterStronglyTypedEnumAttribute, HasAndGetLessEq) {
    auto filter = expression::has_attr<testing::strong_enum>("strong_enum")
            && expression::get_attr<testing::strong_enum>("strong_enum") <= testing::strong_enum::high;
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
    auto filter = expression::has_attr(keyword::severity<ts::severity>());
    log::attributes_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterSeverity, GetEq) {
    auto filter = expression::get_attr(keyword::severity<ts::severity>()) == ts::severity::info;
    log::attributes_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterSeverity, HasAndGetLess) {
    auto filter = expression::has_attr(keyword::severity<ts::severity>())
            && expression::get_attr(keyword::severity<ts::severity>()) < ts::severity::info;
    log::attributes_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    EXPECT_FALSE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::debug };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterSeverity, HasAndGetLessEq) {
    auto filter = expression::has_attr(keyword::severity<ts::severity>())
            && expression::get_attr(keyword::severity<ts::severity>()) <= ts::severity::info;
    log::attributes_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    EXPECT_TRUE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::debug };
    EXPECT_TRUE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::warning };
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterSeverity, HasAndGetGt) {
    auto filter = expression::has_attr(keyword::severity<ts::severity>())
            && expression::get_attr(keyword::severity<ts::severity>()) > ts::severity::info;
    log::attributes_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    EXPECT_FALSE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::debug };
    EXPECT_FALSE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::warning };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterSeverity, HasAndGetGtEq) {
    auto filter = expression::has_attr(keyword::severity<ts::severity>())
            && expression::get_attr(keyword::severity<ts::severity>()) >= ts::severity::info;
    log::attributes_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    EXPECT_TRUE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::debug };
    EXPECT_FALSE(filter(attributes));

    attributes = { keyword::severity<ts::severity>() = ts::severity::warning };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterCustomAttribute, HasOrHas) {
    auto filter = expression::has_attr<std::int32_t>("custom-1") || expression::has_attr<std::int32_t>("custom-2");
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
    auto filter = expression::get_attr<std::int32_t>("custom-1") == 42 &&
            expression::get_attr<std::int32_t>("custom-2") == 100500;
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

TEST(FilterCustomAttribute, GetEqOrGetEq) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") == 42 ||
            expression::get_attr<std::int32_t>("custom-2") == 100500;
    log::attributes_t attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100500)}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100500)}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100501)}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100501)}
    };
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterCustomAttribute, GetLessAndGetLess) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") < 42 &&
            expression::get_attr<std::int32_t>("custom-2") < 100500;
    log::attributes_t attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100500)}
    };
    EXPECT_FALSE(filter(attributes));

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

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100499)}
    };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterCustomAttribute, GetLessOrGetLess) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") < 42 ||
            expression::get_attr<std::int32_t>("custom-2") < 100500;
    log::attributes_t attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100500)}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100500)}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100501)}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100499)}
    };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterCustomAttribute, GetLessEqAndGetLessEq) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") <= 42 &&
            expression::get_attr<std::int32_t>("custom-2") <= 100500;
    log::attributes_t attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100500)}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100500)}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100501)}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100499)}
    };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterCustomAttribute, GetGtAndGetGt) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") > 42 &&
            expression::get_attr<std::int32_t>("custom-2") > 100500;
    log::attributes_t attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100500)}
    };
    EXPECT_FALSE(filter(attributes));

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

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100499)}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(43)},
        {"custom-2", log::attribute_t(100501)}
    };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterCustomAttribute, GetGtEqAndGetGtEq) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") >= 42 &&
            expression::get_attr<std::int32_t>("custom-2") >= 100500;
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
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100499)}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(43)},
        {"custom-2", log::attribute_t(100501)}
    };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterCustomAttribute, TripleAndOperatorWithEqFilter) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") == 42 &&
            expression::get_attr<std::int32_t>("custom-2") == 100500 &&
            expression::get_attr<std::int32_t>("custom-3") == 666;

    log::attributes_t attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100500)},
        {"custom-3", log::attribute_t(666)},
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100500)},
        {"custom-3", log::attribute_t(666)},
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100501)},
        {"custom-3", log::attribute_t(666)},
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100500)},
        {"custom-3", log::attribute_t(667)},
    };
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterCustomAttribute, TripleOrOperatorWithEqFilter) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") == 42 ||
            expression::get_attr<std::int32_t>("custom-2") == 100500 ||
            expression::get_attr<std::int32_t>("custom-3") == 666;

    log::attributes_t attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100500)},
        {"custom-3", log::attribute_t(666)},
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100500)},
        {"custom-3", log::attribute_t(667)},
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100501)},
        {"custom-3", log::attribute_t(666)},
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100501)},
        {"custom-3", log::attribute_t(667)},
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100501)},
        {"custom-3", log::attribute_t(667)},
    };
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterCustomAttribute, CombinationOfLogicOperatorsWithEqFilter) {
    auto filter = (expression::get_attr<std::int32_t>("custom-1") == 42 &&
                   expression::get_attr<std::int32_t>("custom-2") == 100500) ||
            expression::get_attr<std::int32_t>("custom-3") == 666;

    log::attributes_t attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100500)},
        {"custom-3", log::attribute_t(666)},
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100500)},
        {"custom-3", log::attribute_t(666)},
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100501)},
        {"custom-3", log::attribute_t(666)},
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100501)},
        {"custom-3", log::attribute_t(666)},
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100501)},
        {"custom-3", log::attribute_t(667)},
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100500)},
        {"custom-3", log::attribute_t(667)},
    };
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterCustomAttribute, ReversedCombinationOfLogicOperatorsWithEqFilter) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") == 42 ||
            (expression::get_attr<std::int32_t>("custom-2") == 100500 &&
             expression::get_attr<std::int32_t>("custom-3") == 666);

    log::attributes_t attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100500)},
        {"custom-3", log::attribute_t(666)},
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100501)},
        {"custom-3", log::attribute_t(666)},
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(42)},
        {"custom-2", log::attribute_t(100500)},
        {"custom-3", log::attribute_t(667)},
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100500)},
        {"custom-3", log::attribute_t(666)},
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100501)},
        {"custom-3", log::attribute_t(666)},
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {"custom-1", log::attribute_t(41)},
        {"custom-2", log::attribute_t(100500)},
        {"custom-3", log::attribute_t(667)},
    };
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterKeyword, LessThan) {
    auto filter = keyword::severity<level>() < level::info;

    log::attributes_t attributes = {
        {keyword::severity<level>() = level::debug}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::info}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::warn}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::error}
    };
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterKeyword, LessEqThan) {
    auto filter = keyword::severity<level>() <= level::info;

    log::attributes_t attributes = {
        {keyword::severity<level>() = level::debug}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::info}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::warn}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::error}
    };
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterKeyword, GreaterThan) {
    auto filter = keyword::severity<level>() > level::info;

    log::attributes_t attributes = {
        {keyword::severity<level>() = level::debug}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::info}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::warn}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::error}
    };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterKeyword, GreaterEqThan) {
    auto filter = keyword::severity<level>() >= level::info;

    log::attributes_t attributes = {
        {keyword::severity<level>() = level::debug}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::info}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::warn}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::error}
    };
    EXPECT_TRUE(filter(attributes));
}

TEST(FilterKeyword, Eq) {
    auto filter = keyword::severity<level>() == level::info;

    log::attributes_t attributes = {
        {keyword::severity<level>() = level::debug}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::info}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::warn}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::error}
    };
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterKeyword, AndComplex) {
    auto filter =
            keyword::severity<level>() >= level::info &&
            keyword::severity<level>() < level::error;

    log::attributes_t attributes = {
        {keyword::severity<level>() = level::debug}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::info}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::warn}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::error}
    };
    EXPECT_FALSE(filter(attributes));
}

TEST(FilterKeyword, OrComplex) {
    auto filter =
            keyword::severity<level>() < level::info ||
            keyword::severity<level>() == level::error;

    log::attributes_t attributes = {
        {keyword::severity<level>() = level::debug}
    };
    EXPECT_TRUE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::info}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::warn}
    };
    EXPECT_FALSE(filter(attributes));

    attributes = {
        {keyword::severity<level>() = level::error}
    };
    EXPECT_TRUE(filter(attributes));
}
