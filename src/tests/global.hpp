#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <blackhole/utils/unused.hpp>

#define UNUSED(...) \
    blackhole::utils::ignore_unused_variable_warning(__VA_ARGS__);

using namespace ::testing;

namespace testing {

enum level : std::uint32_t { debug, info, warn, error };

} // namespace testing

#define TEST_OFF(__case__, __name__) \
    TEST(__case__, DISABLED_##__name__)
