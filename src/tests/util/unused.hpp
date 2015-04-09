#pragma once

namespace testing { namespace util {

template<typename... Args>
inline
void
ignore(const Args&...) {}

#define UNUSED(...) \
    ::testing::util::ignore(__VA_ARGS__)

}} // namespace testing::util
