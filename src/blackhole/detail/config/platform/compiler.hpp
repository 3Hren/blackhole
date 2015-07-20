#pragma once

#if defined(__APPLE__)
    #include <TargetConditionals.h>
#endif

#if defined(__GNUC__)
    #if __GNUC__ == 4 && __GNUC_MINOR__ == 4
        #define BLACKHOLE_HAS_GCC44
    #endif

    #if __GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
        #define BLACKHOLE_HAS_AT_LEAST_GCC46
    #endif

    #if defined(__clang__)
        #define BLACKHOLE_HAS_CLANG
    #endif
#endif
