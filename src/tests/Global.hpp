#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define UNUSED(...) \
    (void)__VA_ARGS__;

using namespace ::testing;


#define TEST_OFF(__case__, __name__) \
    TEST(__case__, DISABLED_##__name__)

#include "blackhole/error.hpp"
#include "blackhole/expression.hpp"
#include "blackhole/filter.hpp"
#include "blackhole/formatter/json.hpp"
#include "blackhole/formatter/msgpack.hpp"
#include "blackhole/formatter/string.hpp"
#include "blackhole/frontend.hpp"
#include "blackhole/frontend/syslog.hpp"
#include "blackhole/helper.hpp"
#include "blackhole/repository.hpp"
#include "blackhole/keyword.hpp"
#include "blackhole/log.hpp"
#include "blackhole/logger.hpp"
#include "blackhole/sink/files.hpp"
#include "blackhole/sink/socket.hpp"
#include "blackhole/sink/stream.hpp"
#include "blackhole/sink/syslog.hpp"
#include "blackhole/utils/unique.hpp"
