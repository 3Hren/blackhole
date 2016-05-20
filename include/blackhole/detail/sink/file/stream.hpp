#pragma once

#include <iosfwd>
#include <memory>

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {

class stream_factory_t {
public:
    virtual ~stream_factory_t() = default;
    virtual auto create(const std::string& filename, std::ios_base::openmode mode) const ->
        std::unique_ptr<std::ostream> = 0;
};

class ofstream_factory_t : public stream_factory_t {
public:
    virtual auto create(const std::string& filename, std::ios_base::openmode mode) const ->
        std::unique_ptr<std::ostream> override;
};

}  // namespace file
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
