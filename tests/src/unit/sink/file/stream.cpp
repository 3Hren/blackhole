#include <system_error>

#include <gtest/gtest.h>

#include <src/sink/file/stream.hpp>

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {
namespace {

TEST(ofstream_factory_t, ThrowsIfUnableToOpenStream) {
    ofstream_factory_t factory;
    EXPECT_THROW(factory.create("/__mythic/file.log", std::ios_base::app), std::system_error);
}

}  // namespace
}  // namespace file
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
