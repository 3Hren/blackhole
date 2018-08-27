#include "procname.hpp"

#if defined( __linux__ ) || defined( _WIN32 )
#   include <sys/types.h>
#elif __APPLE__
#   include <libproc.h>
#endif

#include <cstring>

#include "blackhole/compat.hpp"
#include "blackhole/stdext/string_view.hpp"

namespace blackhole {
inline namespace v1 {

namespace {

auto procname(pid_t pid) -> stdext::string_view {
#ifdef __linux__
    (void)pid;
    return stdext::string_view(program_invocation_short_name, ::strlen(program_invocation_short_name));
#elif defined(_WIN32)
    static char path[MAX_PATH];
    DWORD       size = sizeof(path);

    if (::QueryFullProcessImageName( GetCurrentProcess(), PROCESS_NAME_NATIVE, path, (PDWORD)&size ) > 0) {
        return stdext::string_view(path, ::strlen(path));
    } else {
        const auto nwritten = ::snprintf(path, sizeof(path), "%d", pid);
        return stdext::string_view(path, static_cast<std::size_t>(nwritten));
    }
#elif __APPLE__
    static char path[PROC_PIDPATHINFO_MAXSIZE];

    if (::proc_name(pid, path, sizeof(path)) > 0) {
        return stdext::string_view(path, ::strlen(path));
    } else {
        const auto nwritten = ::snprintf(path, sizeof(path), "%d", pid);
        return stdext::string_view(path, static_cast<std::size_t>(nwritten));
    }
#endif
}

} // namespace

auto procname() -> stdext::string_view {
    static const stdext::string_view name = procname(::getpid());
    return name;
}

} // namespace v1
} // namespace blackhole
