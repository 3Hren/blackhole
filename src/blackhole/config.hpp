#ifndef CONFIG_H
#define CONFIG_H

#define BLACKHOLE_VERSION_MAJOR 0
#define BLACKHOLE_VERSION_MINOR 2
#define BLACKHOLE_VERSION_PATCH 0
#define BLACKHOLE_DEBUG

#if defined(BLACKHOLE_HEADER_ONLY)
    #define BLACKHOLE_DECL inline
#endif

// If BLACKHOLE_DECL isn't defined yet define it now.
#if !defined(BLACKHOLE_DECL)
    #define BLACKHOLE_DECL
#endif

#endif // CONFIG_H
