#pragma once

#if defined(__clang__) || defined(GCC47)
#else
namespace std {

typedef double double_t;

} // namespace std
#endif
