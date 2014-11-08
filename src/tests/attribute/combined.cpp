#include <functional>

#include <boost/optional.hpp>

#include "blackhole/attribute/name.hpp"
#include "blackhole/attribute/set.hpp"
#include "blackhole/attribute/value.hpp"
#include "blackhole/attribute/view.hpp"

#include "../global.hpp"

namespace blackhole {

namespace attribute {

class combined_view_t {
    std::vector<const set_t*> sets;

public:
    template<class... Other>
    combined_view_t(const set_t& set, const Other&... other) {
        emplace(set, other...);
    }

    template<typename T>
    boost::optional<const T&>
    get(const name_t& name) const {
        if (auto option = get(name)) {
            if (auto result = boost::get<T>(&option.get())) {
                return *result;
            }
        }

        return boost::none;
    }

    boost::optional<const value_t&>
    get(const name_t& name) const {
        const compare_by::name_t action { name };

        for (auto it = this->sets.begin(); it != this->sets.end(); ++it) {
            auto set = *it;
            auto vit = std::find_if(set->begin(), set->end(), action);
            if (vit != set->end()) {
                return vit->second.value;
            }
        }

        return boost::none;
    }

private:
    template<class... Other>
    inline void emplace(const set_t& set, const Other&... other) {
        sets.emplace_back(&set);
        emplace(other...);
    }

    inline void emplace() {}
};

} // namespace attribute

} // namespace blackhole

using namespace blackhole;

TEST(combined_view_t, Ctor) {
    attribute::set_t set;
    attribute::combined_view_t view(set);
    UNUSED(view);
}

TEST(combined_view_t, GetFromSingleSet) {
    attribute::set_t set = {{ "a", attribute_t(42) }};
    attribute::combined_view_t view(set);

    auto attribute = view.get("a");

    ASSERT_TRUE(attribute);
    EXPECT_EQ(42, boost::get<int>(attribute.get()));

    EXPECT_FALSE(view.get("b"));
}

TEST(combined_view_t, GetFromMultipleSets) {
    attribute::set_t set1 = {{ "a1", attribute_t(42) }};
    attribute::set_t set2 = {{ "a2", attribute_t(3.1415) }};
    attribute::set_t set3 = {{ "a3", attribute_t("value") }};
    attribute::combined_view_t view(set1, set2, set3);

    {
        auto attribute = view.get("a1");
        ASSERT_TRUE(attribute);
        EXPECT_EQ(42, boost::get<int>(attribute.get()));
    }

    {
        auto attribute = view.get("a2");
        ASSERT_TRUE(attribute);
        EXPECT_DOUBLE_EQ(3.1415, boost::get<double>(attribute.get()));
    }

    {
        auto attribute = view.get("a3");
        ASSERT_TRUE(attribute);
        EXPECT_EQ("value", boost::get<std::string>(attribute.get()));
    }

    EXPECT_FALSE(view.get("b"));
}

TEST(combined_view_t, TypedGetFromSingleSet) {
    attribute::set_t set = {{ "a", attribute_t(42) }};
    attribute::combined_view_t view(set);

    auto attribute = view.get<int>("a");

    ASSERT_TRUE(attribute);
    EXPECT_EQ(42, attribute.get());

    EXPECT_FALSE(view.get("b"));
    EXPECT_FALSE(view.get<double>("a1"));
}

TEST(combined_view_t, TypedGetFromMultipleSets) {
    attribute::set_t set1 = {{ "a1", attribute_t(42) }};
    attribute::set_t set2 = {{ "a2", attribute_t(3.1415) }};
    attribute::set_t set3 = {{ "a3", attribute_t("value") }};
    attribute::combined_view_t view(set1, set2, set3);

    {
        auto attribute = view.get<int>("a1");
        ASSERT_TRUE(attribute);
        EXPECT_EQ(42, attribute.get());
    }

    {
        auto attribute = view.get<double>("a2");
        ASSERT_TRUE(attribute);
        EXPECT_DOUBLE_EQ(3.1415, attribute.get());
    }

    {
        auto attribute = view.get<std::string>("a3");
        ASSERT_TRUE(attribute);
        EXPECT_EQ("value", attribute.get());
    }

    EXPECT_FALSE(view.get("b"));
    EXPECT_FALSE(view.get<double>("a1"));
}
