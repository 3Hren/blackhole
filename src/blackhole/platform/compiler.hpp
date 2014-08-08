#pragma once

#if defined(__GNUC__)
    #if __GNUC__ == 4 && __GNUC_MINOR__ >= 6
        #define BLACKHOLE_HAVE_AT_LEAST_GCC46
    #endif
#endif
