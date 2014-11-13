#include <stdexcept>

#include <blackhole/expression.hpp>
#include <blackhole/keyword/severity.hpp>
#include <blackhole/keyword/timestamp.hpp>

#include "global.hpp"

using namespace blackhole;

typedef attribute::combined_view_t view_type;

TEST(HasAttribute, ReturnsTrueIfDynamicAttributeExists) {
    auto filter = expression::has_attr<std::int32_t>("custom");
    attribute::set_t attributes = {{"custom", attribute_t(42)}};
    view_type c(attributes);
    EXPECT_TRUE(filter(c));
}

TEST(HasAttribute, ReturnsFalseIfDynamicAttributeNotExists) {
    auto filter = expression::has_attr<std::int32_t>("non-existing-attribute");
    attribute::set_t attributes = {{"custom", attribute_t(42)}};
    view_type c(attributes);
    EXPECT_FALSE(filter(c));
}

TEST(HasAttribute, ReturnsFalseIfDynamicAttributeExistsButTypeMismatched) {
    auto filter = expression::has_attr<std::int32_t>("custom");
    attribute::set_t attributes = {{"custom", attribute_t(3.1415)}};
    view_type c(attributes);
    EXPECT_FALSE(filter(c));
}

TEST(HasAttribute, ReturnsTrueIfBothDynamicAttributeNotExistsAndTypeMismatched) {
    auto filter = expression::has_attr<std::int32_t>("non-existing-attribute");
    attribute::set_t attributes = {{"custom", attribute_t(3.1415)}};
    view_type c(attributes);
    EXPECT_FALSE(filter(c));
}

TEST(FilterCustomAttribute, CanExtractDynamicAttribute) {
    auto filter = expression::get_attr<std::int32_t>("custom");
    attribute::set_t attributes = {{"custom", attribute_t(42)}};
    view_type c(attributes);
    EXPECT_EQ(42, filter(c));
}

TEST(FilterCustomAttribute, ThrowsExceptionIfNameMismatch) {
    /*!
     * It's okay that `get_attr` throws exceptions, cause there is no correct
     * value to be returned if there is no value actually or its type not equal
     * with expected type. You must check `has_attr<T>()` before extracting it.
     * If not, an exception will be thrown and caught inside log core.
     */
    auto filter = expression::get_attr<std::int32_t>("non-existing-attribute");
    attribute::set_t attributes = {{"custom", attribute_t(42)}};
    view_type c(attributes);
    EXPECT_THROW(filter(c), std::logic_error);
}

TEST(FilterCustomAttribute, ThrowsExceptionIfTypeMismatch) {
    auto filter = expression::get_attr<std::int32_t>("custom");
    attribute::set_t attributes = {{"custom", attribute_t(3.1415)}};
    view_type c(attributes);
    EXPECT_THROW(filter(c), std::logic_error);
}

TEST(FilterCustomAttribute, Get) {
    auto filter = expression::get_attr<std::int32_t>("custom") == 42;
    attribute::set_t attributes = {{"custom", attribute_t(42)}};
    view_type c(attributes);
    EXPECT_TRUE(filter(c));
}

TEST(FilterCustomAttribute, FailsIfDeferredlyAttributeComparingFails) {
    auto filter = expression::get_attr<std::int32_t>("custom") == 666;
    attribute::set_t attributes = {{"custom", attribute_t(42)}};
    view_type c(attributes);
    EXPECT_FALSE(filter(c));
}

TEST(FilterCustomAttribute, HasAndGetEq) {
    auto filter = expression::has_attr<std::int32_t>("custom") && expression::get_attr<std::int32_t>("custom") == 42;
    attribute::set_t attributes = {{"custom", attribute_t(42)}};
    view_type c(attributes);
    EXPECT_TRUE(filter(c));
}

TEST(FilterCustomAttribute, HasAndGetLess) {
    auto filter = expression::has_attr<std::int32_t>("custom")
            && expression::get_attr<std::int32_t>("custom") < 42;
    {
        attribute::set_t attributes = {{"custom", attribute_t(42)}};
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {{"custom", attribute_t(41)}};
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

TEST(FilterCustomAttribute, HasAndGetLessEq) {
    auto filter =
        expression::has_attr<std::int32_t>("custom")
        && expression::get_attr<std::int32_t>("custom") <= 42;

    {
        attribute::set_t attributes = {{"custom", attribute_t(42)}};
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {{"custom", attribute_t(41)}};
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {{"custom", attribute_t(43)}};
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }
}

TEST(FilterKeywordAttribute, Get) {
    auto filter = expression::get_attr(keyword::timestamp()) == timeval{ 100500, 0 };
    attribute::set_t attributes = {{"timestamp", attribute_t(timeval{ 100500, 0 })}};
    view_type c(attributes);
    EXPECT_TRUE(filter(c));
}

namespace testing {

enum weak_enum {
    low,
    high
};

} // namespace testing

TEST(FilterWeaklyTypedEnumAttribute, Has) {
    auto filter = expression::has_attr<testing::weak_enum>("weak_enum");
    attribute::set_t attributes = {{ "weak_enum", attribute_t(testing::high) }};
    view_type c(attributes);
    EXPECT_TRUE(filter(c));
}

TEST(FilterWeaklyTypedEnumAttribute, Get) {
    auto filter = expression::get_attr<testing::weak_enum>("weak_enum") == testing::high;
    attribute::set_t attributes = {{ "weak_enum", attribute_t(testing::high) }};
    view_type c(attributes);
    EXPECT_TRUE(filter(c));
}

TEST(FilterWeaklyTypedEnumAttribute, HasAndGetLess) {
    auto filter = expression::has_attr<testing::weak_enum>("weak_enum")
            && expression::get_attr<testing::weak_enum>("weak_enum") < testing::high;
    {
        attribute::set_t attributes = {{ "weak_enum", attribute_t(testing::high) }};
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {{ "weak_enum", attribute_t(testing::low) }};
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

TEST(FilterWeaklyTypedEnumAttribute, HasAndGetLessEq) {
    auto filter = expression::has_attr<testing::weak_enum>("weak_enum")
            && expression::get_attr<testing::weak_enum>("weak_enum") <= testing::high;

    {
        attribute::set_t attributes = {{ "weak_enum", attribute_t(testing::high) }};
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {{ "weak_enum", attribute_t(testing::low) }};
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

namespace testing {

enum class strong_enum {
    low,
    high
};

} // namespace testing

TEST(FilterStronglyTypedEnumAttribute, Has) {
    auto filter = expression::has_attr<testing::strong_enum>("strong_enum");
    attribute::set_t attributes = {{ "strong_enum", attribute_t(testing::strong_enum::high) }};
    view_type c(attributes);
    EXPECT_TRUE(filter(c));
}

TEST(FilterStronglyTypedEnumAttribute, Get) {
    auto filter = expression::get_attr<testing::strong_enum>("strong_enum") == testing::strong_enum::high;
    attribute::set_t attributes = {{ "strong_enum", attribute_t(testing::strong_enum::high) }};
    view_type c(attributes);
    EXPECT_TRUE(filter(c));
}

TEST(FilterStronglyTypedEnumAttribute, HasAndGetLess) {
    auto filter = expression::has_attr<testing::strong_enum>("strong_enum")
            && expression::get_attr<testing::strong_enum>("strong_enum") < testing::strong_enum::high;

    {
        attribute::set_t attributes = {{ "strong_enum", attribute_t(testing::strong_enum::high) }};
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {{ "strong_enum", attribute_t(testing::strong_enum::low) }};
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

TEST(FilterStronglyTypedEnumAttribute, HasAndGetLessEq) {
    auto filter = expression::has_attr<testing::strong_enum>("strong_enum")
            && expression::get_attr<testing::strong_enum>("strong_enum") <= testing::strong_enum::high;
    {
        attribute::set_t attributes = {{ "strong_enum", attribute_t(testing::strong_enum::high) }};
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {{ "strong_enum", attribute_t(testing::strong_enum::low) }};
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
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
    attribute::set_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    view_type c(attributes);
    EXPECT_TRUE(filter(c));
}

TEST(FilterSeverity, GetEq) {
    auto filter = expression::get_attr(keyword::severity<ts::severity>()) == ts::severity::info;
    attribute::set_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
    view_type c(attributes);
    EXPECT_TRUE(filter(c));
}

TEST(FilterSeverity, HasAndGetLess) {
    auto filter = expression::has_attr(keyword::severity<ts::severity>())
            && expression::get_attr(keyword::severity<ts::severity>()) < ts::severity::info;

    {
        attribute::set_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = { keyword::severity<ts::severity>() = ts::severity::debug };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

TEST(FilterSeverity, HasAndGetLessEq) {
    auto filter = expression::has_attr(keyword::severity<ts::severity>())
            && expression::get_attr(keyword::severity<ts::severity>()) <= ts::severity::info;

    {
        attribute::set_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = { keyword::severity<ts::severity>() = ts::severity::debug };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = { keyword::severity<ts::severity>() = ts::severity::warning };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }
}

TEST(FilterSeverity, HasAndGetGt) {
    auto filter = expression::has_attr(keyword::severity<ts::severity>())
            && expression::get_attr(keyword::severity<ts::severity>()) > ts::severity::info;

    {
        attribute::set_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = { keyword::severity<ts::severity>() = ts::severity::debug };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = { keyword::severity<ts::severity>() = ts::severity::warning };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

TEST(FilterSeverity, HasAndGetGtEq) {
    auto filter = expression::has_attr(keyword::severity<ts::severity>())
            && expression::get_attr(keyword::severity<ts::severity>()) >= ts::severity::info;

    {
        attribute::set_t attributes = { keyword::severity<ts::severity>() = ts::severity::info };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = { keyword::severity<ts::severity>() = ts::severity::debug };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = { keyword::severity<ts::severity>() = ts::severity::warning };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

TEST(FilterCustomAttribute, HasOrHas) {
    auto filter = expression::has_attr<std::int32_t>("custom-1") || expression::has_attr<std::int32_t>("custom-2");

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100501)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-0", attribute_t(42)},
            {"custom-1", attribute_t(100501)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-0", attribute_t(41)},
            {"custom-3", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }
}

TEST(FilterCustomAttribute, GetEqAndGetEq) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") == 42 &&
            expression::get_attr<std::int32_t>("custom-2") == 100500;

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100501)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }
}

TEST(FilterCustomAttribute, GetEqOrGetEq) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") == 42 ||
            expression::get_attr<std::int32_t>("custom-2") == 100500;

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100501)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100501)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }
}

TEST(FilterCustomAttribute, GetLessAndGetLess) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") < 42 &&
            expression::get_attr<std::int32_t>("custom-2") < 100500;
    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100501)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100499)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

TEST(FilterCustomAttribute, GetLessOrGetLess) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") < 42 ||
            expression::get_attr<std::int32_t>("custom-2") < 100500;
    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100501)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100499)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

TEST(FilterCustomAttribute, GetLessEqAndGetLessEq) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") <= 42 &&
            expression::get_attr<std::int32_t>("custom-2") <= 100500;

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100501)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100499)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

TEST(FilterCustomAttribute, GetGtAndGetGt) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") > 42 &&
            expression::get_attr<std::int32_t>("custom-2") > 100500;

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100501)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100499)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(43)},
            {"custom-2", attribute_t(100501)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

TEST(FilterCustomAttribute, GetGtEqAndGetGtEq) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") >= 42 &&
            expression::get_attr<std::int32_t>("custom-2") >= 100500;

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100500)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100501)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100499)}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(43)},
            {"custom-2", attribute_t(100501)}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

TEST(FilterCustomAttribute, TripleAndOperatorWithEqFilter) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") == 42 &&
            expression::get_attr<std::int32_t>("custom-2") == 100500 &&
            expression::get_attr<std::int32_t>("custom-3") == 666;

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100500)},
            {"custom-3", attribute_t(666)},
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100500)},
            {"custom-3", attribute_t(666)},
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100501)},
            {"custom-3", attribute_t(666)},
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100500)},
            {"custom-3", attribute_t(667)},
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }
}

TEST(FilterCustomAttribute, TripleOrOperatorWithEqFilter) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") == 42 ||
            expression::get_attr<std::int32_t>("custom-2") == 100500 ||
            expression::get_attr<std::int32_t>("custom-3") == 666;
    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100500)},
            {"custom-3", attribute_t(666)},
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100500)},
            {"custom-3", attribute_t(667)},
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100501)},
            {"custom-3", attribute_t(666)},
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100501)},
            {"custom-3", attribute_t(667)},
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100501)},
            {"custom-3", attribute_t(667)},
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }
}

TEST(FilterCustomAttribute, CombinationOfLogicOperatorsWithEqFilter) {
    auto filter = (expression::get_attr<std::int32_t>("custom-1") == 42 &&
                   expression::get_attr<std::int32_t>("custom-2") == 100500) ||
            expression::get_attr<std::int32_t>("custom-3") == 666;

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100500)},
            {"custom-3", attribute_t(666)},
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100500)},
            {"custom-3", attribute_t(666)},
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100501)},
            {"custom-3", attribute_t(666)},
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100501)},
            {"custom-3", attribute_t(666)},
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100501)},
            {"custom-3", attribute_t(667)},
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100500)},
            {"custom-3", attribute_t(667)},
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }
}

TEST(FilterCustomAttribute, ReversedCombinationOfLogicOperatorsWithEqFilter) {
    auto filter = expression::get_attr<std::int32_t>("custom-1") == 42 ||
            (expression::get_attr<std::int32_t>("custom-2") == 100500 &&
             expression::get_attr<std::int32_t>("custom-3") == 666);

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100500)},
            {"custom-3", attribute_t(666)},
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100501)},
            {"custom-3", attribute_t(666)},
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(42)},
            {"custom-2", attribute_t(100500)},
            {"custom-3", attribute_t(667)},
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100500)},
            {"custom-3", attribute_t(666)},
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100501)},
            {"custom-3", attribute_t(666)},
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {"custom-1", attribute_t(41)},
            {"custom-2", attribute_t(100500)},
            {"custom-3", attribute_t(667)},
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }
}

TEST(FilterKeyword, LessThan) {
    auto filter = keyword::severity<level>() < level::info;

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::debug}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::info}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::warn}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::error}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }
}

TEST(FilterKeyword, LessEqThan) {
    auto filter = keyword::severity<level>() <= level::info;

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::debug}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::info}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::warn}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::error}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }
}

TEST(FilterKeyword, GreaterThan) {
    auto filter = keyword::severity<level>() > level::info;

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::debug}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::info}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::warn}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::error}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

TEST(FilterKeyword, GreaterEqThan) {
    auto filter = keyword::severity<level>() >= level::info;

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::debug}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::info}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::warn}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::error}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}

TEST(FilterKeyword, Eq) {
    auto filter = keyword::severity<level>() == level::info;

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::debug}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::info}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::warn}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::error}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }
}

TEST(FilterKeyword, AndComplex) {
    auto filter =
            keyword::severity<level>() >= level::info &&
            keyword::severity<level>() < level::error;

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::debug}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::info}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::warn}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::error}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }
}

TEST(FilterKeyword, OrComplex) {
    auto filter =
            keyword::severity<level>() < level::info ||
            keyword::severity<level>() == level::error;

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::debug}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::info}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::warn}
        };
        view_type c(attributes);
        EXPECT_FALSE(filter(c));
    }

    {
        attribute::set_t attributes = {
            {keyword::severity<level>() = level::error}
        };
        view_type c(attributes);
        EXPECT_TRUE(filter(c));
    }
}
