#include "blackhole/detail/procname.hpp"

#if defined(__linux__)
#   include <sys/types.h>
#	include <unistd.h>
#elif defined(__APPLE__)
#   include <libproc.h>
#	include <unistd.h>
#elif defined(_WIN32)
#endif

#include <cstring>

#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {
inline namespace v1 {
namespace detail {

using cpp17::string_view;

namespace {

auto __procname() -> string_view {
#if defined(__linux__)
	return string_view(program_invocation_short_name, ::strlen(program_invocation_short_name));
#elif defined(__APPLE__)
	const auto pid = ::getpid();

	static char path[PROC_PIDPATHINFO_MAXSIZE];

	if (::proc_name(pid, path, sizeof(path)) > 0) {
		return string_view(path, ::strlen(path));
	} else {
		const auto nwritten = ::snprintf(path, sizeof(path), "%d", pid);
		return string_view(path, static_cast<std::size_t>(nwritten));
	}
#elif defined(_WIN32)
	// TODO: Implement.
	return "";
#endif
}

}  // namespace

auto procname() -> string_view {
    static const string_view name = __procname();
    return name;
}

}  // namespace detail
}  // namespace v1
}  // namespace blackhole
