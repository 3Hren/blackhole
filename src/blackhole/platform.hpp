#pragma once

#if defined(__GNUC__)
    #if __GNUC__ == 4 && __GNUC_MINOR__ >= 4
        #define HAVE_GCC44
    #endif

    #if __GNUC__ == 4 && __GNUC_MINOR__ >= 6
        #define HAVE_GCC46
    #endif

    #if __GNUC__ == 4 && __GNUC_MINOR__ >= 7
        #define HAVE_GCC47
    #endif
#endif
