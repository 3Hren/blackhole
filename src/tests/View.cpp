#include <vector>
#include <unordered_map>

#include <blackhole/attribute/view.hpp>

#include "global.hpp"

using namespace blackhole;
using aux::iterator::join_t;
using aux::iterator::invalidate_tag;

#define TEST_JOIN_ITERATOR(Suite, Case) \
    TEST(join_t##_##Suite, Case)

TEST_JOIN_ITERATOR(Cons, Converting) {
    typedef std::vector<int> container_type;
    container_type c1, c2;

    join_t<container_type, true> it({ &c1, &c2 });
    UNUSED(it);
}

TEST_JOIN_ITERATOR(Cons, Invalidation) {
    typedef std::vector<int> container_type;
    container_type c1, c2;

    join_t<container_type, true> it({ &c1, &c2 }, invalidate_tag);
    UNUSED(it);
}

TEST_JOIN_ITERATOR(Cons, Copy) {
    typedef std::vector<int> container_type;
    container_type c1 = { 0, 1, 2 };
    container_type c2 = { 3, 4, 5 };

    join_t<container_type, true> it1({ &c1, &c2 });
    join_t<container_type, true> it2 = it1;
    EXPECT_TRUE(it1 == it2);

    container_type expected = { 0, 1, 2, 3, 4, 5 };
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), it2));
}

TEST_JOIN_ITERATOR(Cons, Move) {
    typedef std::vector<int> container_type;
    container_type c1 = { 0, 1, 2 };
    container_type c2 = { 3, 4, 5 };

    join_t<container_type, true> it1({ &c1, &c2 });
    join_t<container_type, true> it2 = std::move(it1);

    container_type expected = { 0, 1, 2, 3, 4, 5 };
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), it2));
}

TEST_JOIN_ITERATOR(Equality, EmptyContainers) {
    typedef std::vector<int> container_type;
    container_type c1, c2;

    join_t<container_type, true> it1({ &c1, &c2 });
    join_t<container_type, true> it2({ &c1, &c2 });
    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 != it2);
}

TEST_JOIN_ITERATOR(Equality, EqualContainers) {
    typedef std::vector<int> container_type;
    container_type c1 = { 0, 1, 2 };
    container_type c2 = { 0, 1, 2 };

    join_t<container_type, true> it1({ &c1, &c2 });
    join_t<container_type, true> it2({ &c1, &c2 });
    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 != it2);
}

TEST_JOIN_ITERATOR(Equality, NotEqualContainers) {
    typedef std::vector<int> container_type;
    container_type c1 = { 0, 1, 2 };
    container_type c2 = { 0, 1, 2 };

    join_t<container_type, true> it1({ &c1, &c2 });
    join_t<container_type, true> it2({ &c2, &c1 });
    EXPECT_FALSE(it1 == it2);
    EXPECT_TRUE(it1 != it2);
}

TEST_JOIN_ITERATOR(Forward, EmptyFirst) {
    typedef std::vector<int> container_type;
    container_type c1 = {};
    container_type c2 = { 3, 4, 5 };

    join_t<container_type, true> it({ &c1, &c2 });
    container_type expected = { 3, 4, 5 };
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), it));
}

TEST_JOIN_ITERATOR(Forward, EmptyMiddle) {
    typedef std::vector<int> container_type;
    container_type c1 = { 0, 1, 2 };
    container_type c2 = {};
    container_type c3 = { 3, 4, 5 };

    join_t<container_type, true> it({ &c1, &c2, &c3 });
    container_type expected = { 0, 1, 2, 3, 4, 5 };
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), it));
}

TEST_JOIN_ITERATOR(Forward, EmptyLast) {
    typedef std::vector<int> container_type;
    container_type c1 = { 0, 1, 2 };
    container_type c2 = {};

    join_t<container_type, true> it({ &c1, &c2 });
    container_type expected = { 0, 1, 2 };
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), it));
}

TEST_JOIN_ITERATOR(Forward, Loop) {
    typedef std::vector<int> container_type;
    container_type c1 = { 0, 1, 2 };
    container_type c2 = { 3, 4, 5 };

    join_t<container_type, true> begin({ &c1, &c2 });
    join_t<container_type, true> end({ &c1, &c2 }, invalidate_tag);
    container_type expected = { 0, 1, 2, 3, 4, 5 };
    size_t s = 0;
    for (auto it = begin; it != end; ++it) {
        EXPECT_EQ(expected[s++], *it);
    }
}

TEST_JOIN_ITERATOR(Forward, LoopPostIncrement) {
    typedef std::vector<int> container_type;
    container_type c1 = { 0, 1, 2 };
    container_type c2 = { 3, 4, 5 };

    join_t<container_type, true> begin({ &c1, &c2 });
    join_t<container_type, true> end({ &c1, &c2 }, invalidate_tag);
    container_type expected = { 0, 1, 2, 3, 4, 5 };
    size_t s = 0;
    for (auto it = begin; it != end; it++) {
        EXPECT_EQ(expected[s++], *it);
    }
}

TEST_JOIN_ITERATOR(Forward, Map) {
    typedef std::unordered_map<std::string, int> container_type;
    container_type c1 = { {"0", 0}, {"1", 1}, {"2", 2} };
    container_type c2 = { {"3", 0}, {"4", 1}, {"5", 2} };

    const join_t<container_type, true> begin({ &c1, &c2 });
    join_t<container_type, true> end({ &c1, &c2 }, invalidate_tag);
    container_type expected = {
        {"0", 0}, {"1", 1}, {"2", 2}, {"3", 0}, {"4", 1}, {"5", 2}
    };

    ASSERT_EQ(expected.size(), std::distance(begin, end));
    for (auto it = begin; it != end; ++it) {
        EXPECT_TRUE(expected.find(it->first)->second == it->second);
    }
}

TEST(set_view_t, DefaultConstructor) {
    attribute::set_view_t view;
    EXPECT_TRUE(view.empty());
}
