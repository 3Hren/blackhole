#pragma once

#define BLACKHOLE_DECLARE_NONCOPYABLE(__type__) \
    __type__(const __type__& other) = delete; \
    __type__& operator=(const __type__& other) = delete
