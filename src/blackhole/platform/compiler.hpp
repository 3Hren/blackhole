#pragma once

#if defined(__GNUC__)
    #if __GNUC__ == 4 && __GNUC_MINOR__ >= 4
        #define HAVE_GCC44
        #define BH_HAVE_AT_LEAST_GCC_44
    #endif

    #if __GNUC__ == 4 && __GNUC_MINOR__ >= 6
        #define HAVE_GCC46
        #define BH_HAVE_AT_LEAST_GCC_46
    #endif

    #if __GNUC__ == 4 && __GNUC_MINOR__ >= 7
        #define HAVE_GCC47
        #define BH_HAVE_AT_LEAST_GCC_47
    #endif
#endif
