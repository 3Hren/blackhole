#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define UNUSED(...) \
    (void)__VA_ARGS__;

using namespace ::testing;


#define TEST_OFF(__case__, __name__) \
    TEST(__case__, DISABLED_##__name__)
