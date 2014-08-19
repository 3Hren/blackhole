#include <tuple>
#include <vector>
#include <unordered_map>

#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>

#include <blackhole/attribute/view.hpp>

#include "global.hpp"

using namespace blackhole::attribute;

TEST(join_iterator_t, DefaultConstructor) {
    typedef std::vector<int> container_type;
    join_iterator_t<container_type, true> it;
    UNUSED(it);
}

TEST(join_iterator_t, EmptyIteratorEqualsIteratorWithEmptyContainers) {
    typedef std::vector<int> container_type;
    container_type c1, c2;

    join_iterator_t<container_type, true> it1;
    join_iterator_t<container_type, true> it2({ &c1, &c2 });
    EXPECT_TRUE(it1 == it2);
}

TEST(join_iterator_t, ConvertingConstructor) {
    typedef std::vector<int> container_type;
    container_type c1, c2;

    join_iterator_t<container_type, true> it({ &c1, &c2 });
    UNUSED(it);
}

TEST(join_iterator_t, Forward) {
    typedef std::vector<int> container_type;
    container_type c1 = { 0, 1, 2 };
    container_type c2 = { 3, 4, 5 };

    join_iterator_t<container_type, true> it({ &c1, &c2 });
    container_type expected = { 0, 1, 2, 3, 4, 5 };
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), it));
}

TEST(join_iterator_t, ForwardWithEmptyFirst) {
    typedef std::vector<int> container_type;
    container_type c1 = {};
    container_type c2 = { 3, 4, 5 };

    join_iterator_t<container_type, true> it({ &c1, &c2 });
    container_type expected = { 3, 4, 5 };
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), it));
}

TEST(join_iterator_t, ForwardWithEmptyLast) {
    typedef std::vector<int> container_type;
    container_type c1 = { 1, 2, 3 };
    container_type c2 = {};

    join_iterator_t<container_type, true> it({ &c1, &c2 });
    container_type expected = { 1, 2, 3 };
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), it));
}

TEST(join_iterator_t, ForwardLoop) {
    typedef std::vector<int> container_type;
    container_type c1 = { 0, 1, 2 };
    container_type c2 = { 3, 4, 5 };

    join_iterator_t<container_type, true> begin({ &c1, &c2 });
    join_iterator_t<container_type, true> end({ &c1, &c2 }, invalidate_tag);
    container_type expected = { 0, 1, 2, 3, 4, 5 };
    size_t s = 0;
    for (auto it = begin; it != end; ++it) {
        EXPECT_EQ(expected[s++], *it);
    }
}

TEST(join_iterator_t, ForwardLoopPostIncrement) {
    typedef std::vector<int> container_type;
    container_type c1 = { 0, 1, 2 };
    container_type c2 = { 3, 4, 5 };

    join_iterator_t<container_type, true> begin({ &c1, &c2 });
    join_iterator_t<container_type, true> end({ &c1, &c2 }, invalidate_tag);
    container_type expected = { 0, 1, 2, 3, 4, 5 };
    size_t s = 0;
    for (auto it = begin; it != end; it++) {
        EXPECT_EQ(expected[s++], *it);
    }
}

TEST(join_iterator_t, ForwardMap) {
    typedef std::unordered_map<std::string, int> container_type;
    container_type c1 = { {"0", 0}, {"1", 1}, {"2", 2} };
    container_type c2 = { {"3", 0}, {"4", 1}, {"5", 2} };

    const join_iterator_t<container_type, true> begin({ &c1, &c2 });
    join_iterator_t<container_type, true> end({ &c1, &c2 }, invalidate_tag);
    container_type expected = {
        {"0", 0}, {"1", 1}, {"2", 2}, {"3", 0}, {"4", 1}, {"5", 2}
    };

    ASSERT_EQ(expected.size(), std::distance(begin, end));
    for (auto it = begin; it != end; ++it) {
        EXPECT_TRUE(expected.find(it->first)->second == it->second);
    }
}
