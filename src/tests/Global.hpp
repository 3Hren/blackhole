#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define UNUSED(T...) \
    (void)T;

using namespace ::testing;


#define TEST_OFF(__case__, __name__) \
    TEST(__case__, DISABLED_##__name__)

#include "blackhole/log.hpp"
