#pragma once

#include <blackhole/sink/files.hpp>

namespace testing {

namespace mock {

namespace files {

class backend_t {
public:
    backend_t(const std::string& = std::string()) {
        init();
    }

    void init() {
        ON_CALL(*this, opened())
                .WillByDefault(Return(true));
        ON_CALL(*this, exists(_))
                .WillByDefault(Return(false));
        ON_CALL(*this, listdir())
                .WillByDefault(Return(std::vector<std::string>()));
        ON_CALL(*this, changed(_))
                .WillByDefault(Return(std::time(nullptr)));
    }

    MOCK_CONST_METHOD1(exists, bool(const std::string&));
    MOCK_CONST_METHOD0(opened, bool());
    MOCK_CONST_METHOD0(path, std::string());
    MOCK_CONST_METHOD0(filename, std::string());
    MOCK_CONST_METHOD0(listdir, std::vector<std::string>());
    MOCK_CONST_METHOD1(changed, std::time_t(const std::string&));
    MOCK_CONST_METHOD1(size, std::uint64_t(const std::string&));

    MOCK_CONST_METHOD0(open, bool());
    MOCK_METHOD2(rename, void(const std::string&, const std::string&));
    MOCK_CONST_METHOD1(write, void(const std::string&));
    MOCK_METHOD0(close, void());
    MOCK_CONST_METHOD0(flush, void());
};

namespace rotation {

class watcher_t {
public:
    watcher_t(const blackhole::sink::rotation::watcher::config_t<watcher_t>&) {}
};

} // namespace rotation

} // namespace files

} // namespace mock

} // namespace testing
