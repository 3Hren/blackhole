#ifndef CONFIG_H
#define CONFIG_H

#define BLACKHOLE_VERSION_MAJOR 0
#define BLACKHOLE_VERSION_MINOR 2
#define BLACKHOLE_VERSION_PATCH 0
#define BLACKHOLE_HEADER_ONLY
#define BLACKHOLE_DEBUG

#if defined(BLACKHOLE_HEADER_ONLY)
    #define BLACKHOLE_API inline
#endif

// If BLACKHOLE_API isn't defined yet define it now.
#if !defined(BLACKHOLE_API)
    #define BLACKHOLE_API
#endif

#endif // CONFIG_H
