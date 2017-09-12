#pragma once

#include <sys/stat.h>

#include <string>
#include <system_error>

#include "../rotate.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {
namespace rotate {

class stat_rotate_t : public rotate_t {
    std::string filename;
    std::int64_t inode;

public:
    explicit stat_rotate_t(std::string filename) :
        filename(std::move(filename)),
        inode(0)
    {
        struct stat buf = {};
        auto rc = ::stat(this->filename.c_str(), &buf);
        if (rc == -1) {
            throw std::system_error(errno, std::system_category());
        }

        inode = std::int64_t(buf.st_ino);
    }

    auto should_rotate() -> bool override {
        struct stat buf = {};
        auto rc = ::stat(filename.c_str(), &buf);

        return !(rc == 0 && std::int64_t(buf.st_ino) == inode);
    }
};

class stat_factory_t : public rotate_factory_t {
public:
    auto create(const std::string& filename) const -> std::unique_ptr<rotate_t> override {
        return blackhole::make_unique<stat_rotate_t>(filename);
    }
};

} // namespace rotate
} // namespace file
} // namespace sink
} // namespace v1
} // namespace blackhole
