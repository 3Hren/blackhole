#pragma once

#if defined(__clang__) || defined(GCC47) || __GNUC__ >= 5
#if __GNUC__ >= 5
// It is needed to resolve `<cmath>` header which requires `std::double_t`.
#include <math.h>
#endif

#else
namespace std {

typedef double double_t;

} // namespace std
#endif
