#pragma once

#include <string>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/version.hpp>

namespace blackhole {

namespace sink {

namespace files {

class boost_backend_t {
    const boost::filesystem::path m_path;
    boost::filesystem::ofstream m_file;
public:
    boost_backend_t(const std::string& path) :
        m_path(absolute(path))
    {
    }

    bool opened() const {
        return m_file.is_open();
    }

    bool exists(const std::string& filename) const {
        return boost::filesystem::exists(m_path.parent_path() / filename);
    }

    std::vector<std::string> listdir() const {
        std::vector<std::string> result;
        for (boost::filesystem::directory_iterator it(m_path.parent_path());
             it != boost::filesystem::directory_iterator();
             ++it) {
            result.push_back(filename(it->path()));
        }

        return result;
    }

    std::time_t changed(const std::string& filename) const {
        return boost::filesystem::last_write_time(m_path.parent_path() / filename);
    }

    std::uint64_t size(const std::string& filename) const {
        return boost::filesystem::file_size(m_path.parent_path() / filename);
    }

    bool open() {
        m_file.open(m_path, std::ios_base::out | std::ios_base::app);
        return m_file.is_open();
    }

    void close() {
        m_file.close();
    }

    void rename(const std::string& oldname, const std::string& newname) {
        if (oldname == newname) {
            return;
        }

        const boost::filesystem::path& path = m_path.parent_path();
        const boost::filesystem::path& oldpath = path / oldname;
        const boost::filesystem::path& newpath = path / newname;

        //! Workaround `boost::filesystem::rename`: boost < 1.46.00 throws exception
        //! if target path exists.
#if BOOST_VERSION < 104600 || BOOST_FILESYSTEM_VERSION < 3
        if (boost::filesystem::exists(newpath)) {
            boost::filesystem::remove(newpath);
        }
#endif
        boost::filesystem::rename(oldpath, newpath);
    }

    std::string filename() const {
        return filename(m_path);
    }

    std::string path() const {
        return m_path.string();
    }

    void write(const std::string& message) {
        m_file.write(message.data(), static_cast<std::streamsize>(message.size()));
        m_file.put('\n');
    }

    void flush() {
        m_file.flush();
    }

private:
    static inline boost::filesystem::path initial_path() {
        return boost::filesystem::initial_path<boost::filesystem::path>();
    }

    static inline std::string filename(const boost::filesystem::path& path) {
#if BOOST_VERSION >= 104600 && BOOST_FILESYSTEM_VERSION >= 3
        return path.filename().string();
#else
        return path.filename();
#endif
    }

    static inline boost::filesystem::path absolute(const boost::filesystem::path& path) {
#if BOOST_VERSION >= 104600 && BOOST_FILESYSTEM_VERSION >= 3
        return boost::filesystem::absolute(path, initial_path());
#else
        return boost::filesystem::complete(path, initial_path());
#endif
    }
};

} // namespace files

} // namespace sink

} // namespace blackhole
