#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define UNUSED(T...) \
    (void)T;

using namespace ::testing;


#define TEST_OFF(__case__, __name__) \
    TEST(__case__, DISABLED_##__name__)

#include "blackhole/error.hpp"
#include "blackhole/expression.hpp"
#include "blackhole/filter.hpp"
#include "blackhole/formatter/string.hpp"
#include "blackhole/frontend.hpp"
#include "blackhole/helper.hpp"
#include "blackhole/keyword.hpp"
#include "blackhole/logger.hpp"
#include "blackhole/sink/files.hpp"
#include "blackhole/sink/socket.hpp"
#include "blackhole/sink/syslog.hpp"
#include "blackhole/utils/unique.hpp"
