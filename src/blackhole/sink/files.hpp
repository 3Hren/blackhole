#pragma once

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "blackhole/error.hpp"

namespace blackhole {

namespace sink {

class boost_backend_t {
    boost::filesystem::path m_path;
    boost::filesystem::ofstream m_file;
public:
    boost_backend_t(const std::string& path) :
        m_path(path)
    {
    }

    bool opened() const {
        return m_file.is_open();
    }

    bool open() {
        m_file.open(m_path, std::ios_base::out | std::ios_base::app);
        return m_file.is_open();
    }

    std::string path() const {
        return m_path.filename().string();
    }

    void write(const std::string& message) {
        m_file.write(message.data(), static_cast<std::streamsize>(message.size()));
        m_file.put('\n');
        m_file.flush();
    }
};

template<class Backend = boost_backend_t>
class file_t {
    Backend m_backend;
public:
    file_t(const std::string& path) :
        m_backend(path)
    {
    }

    void consume(const std::string& message) {
        //!@todo: if (m_backend.need_rotate(message.size()) m_backend.rotate();

        if (!m_backend.opened()) {
            //!@todo: create directory if not exists
            if (!m_backend.open()) {
                throw error_t("failed to open file '%s' for writing", m_backend.path());
            }
        }
        m_backend.write(message);
    }

    Backend& backend() {
        return m_backend;
    }
};

} // namespace sink

} // namespace blackhole
