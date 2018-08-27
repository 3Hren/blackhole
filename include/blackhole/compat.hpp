# pragma once
# if !defined( __blackhole_compat_hpp__ )
# define __blackhole_compat_hpp__

# include <algorithm>

# if defined( _MSC_VER )
#   define NOMINMAX

#   include <windows.h>
#   include <time.h>
#   include <io.h>

typedef DWORD   pid_t;
typedef void*   pthread_t;

# define  getpid        GetCurrentProcessId

inline void* pthread_self()
  {
    using uintcast_t = std::conditional<sizeof(void*) == sizeof(DWORD), uint32_t, uint64_t>::type;
    
    DWORD thread = GetCurrentThreadId();
    return (void*)(uintcast_t)thread;
  }

inline struct tm* gmtime_r( const time_t *timep, struct tm *result)
  {
    return (gmtime_s( result, timep ), result);
  }
  
inline struct tm *localtime_r(const time_t *timep, struct tm *result)
  {
    return (localtime_s( result, timep ), result);
  }

inline int  pthread_getname_np(pthread_t thread, char *name, size_t len)
  {
    PWSTR   descr;
    int     bytes;
    
    if ( FAILED( GetThreadDescription( thread, &descr ) ) )
      return EFAULT;

    bytes = WideCharToMultiByte( CP_UTF8, 0, descr, -1, name, (int)len, NULL, NULL );

    LocalFree( descr );

    return bytes == 0 ? ERANGE : 0;
  }

# define tzset    _tzset
# define timezone _timezone

# else
#   include <unistd.h>
# endif

# if defined( _MSC_VER )
#   define  __attribute__( expression )
#   define  NORETURN  __declspec( noreturn )
# else
#   define  NORETURN  __attribute((noreturn))
# endif

# endif   // !__blackhole_compat_hpp__
