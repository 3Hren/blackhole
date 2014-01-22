#pragma once

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/version.hpp>

#include "blackhole/error.hpp"
#include "blackhole/factory.hpp"

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
        if (!create_directories(m_path.parent_path())) {
            return false;
        }

        m_file.open(m_path, std::ios_base::out | std::ios_base::app);
        return m_file.is_open();
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
    template<typename Path>
    bool create_directories(const Path& path) {
        try {
            boost::filesystem::create_directories(path);
        } catch (const boost::filesystem::filesystem_error&) {
            return false;
        }

        return true;
    }
};

struct null_rotator_t {
    static const char* name() {
        return "";
    }
};

struct rotator_t {
    static const char* name() {
        return "/rotate";
    }
};

namespace file {

struct rotator_config_t {
    std::uint64_t size;
    std::uint16_t count;
};

template<typename Rotator = null_rotator_t>
struct config_t {
    std::string path;
    bool autoflush;

    config_t(const std::string& path, bool autoflush) :
        path(path),
        autoflush(autoflush)
    {}

    config_t(const std::string& path) :
        path(path),
        autoflush(true)
    {}

    config_t() :
        path("/dev/stdout"),
        autoflush(true)
    {}
};

template<>
struct config_t<rotator_t> {
    std::string path;
    bool autoflush;
    rotator_config_t rotator;

    config_t(const std::string& path, bool autoflush) :
        path(path),
        autoflush(autoflush)
    {}

    config_t(const std::string& path) :
        path(path),
        autoflush(true)
    {}

    config_t() :
        path("/dev/stdout"),
        autoflush(true)
    {}
};

} // namespace file

template<class Backend = boost_backend_t, class Rotator = null_rotator_t>
class file_t {
    file::config_t<Rotator> config;
    Backend m_backend;
    Rotator m_rotator;
public:
    typedef file::config_t<Rotator> config_type;

    static const char* name() {
        return "files";
    }

    file_t(const std::string& path) :
        config(path),
        m_backend(path)
    {}

    file_t(const config_type& config) :
        config(config),
        m_backend(config.path)
    {}

    void consume(const std::string& message) {
        if (!m_backend.opened()) {
            if (!m_backend.open()) {
                throw error_t("failed to open file '%s' for writing", m_backend.path());
            }
        }
        m_backend.write(message);

        if (config.autoflush) {
            m_backend.flush();
        }
    }

    Backend& backend() {
        return m_backend;
    }
};

} // namespace sink

template<class Backend, class Rotator>
struct config_traits<sink::file_t<Backend, Rotator>> {
    static std::string name() {
        return utils::format("files%s", Rotator::name());
    }

    static std::string parse(const boost::any& config) {
        std::vector<boost::any> cfg;
        aux::any_to(config, cfg);
        std::string rotator;

        const uint ROTATOR_POS = 2;
        if (cfg.size() > ROTATOR_POS && aux::is<std::vector<boost::any>>(cfg.at(ROTATOR_POS))) {
            rotator = sink::rotator_t::name();
        }

        return utils::format("files%s", rotator);
    }
};

template<class Backend>
struct factory_traits<sink::file_t<Backend>> {
    typedef typename sink::file_t<Backend>::config_type config_type;

    static config_type map_config(const boost::any& config) {
        config_type cfg;
        aux::vector_to(config, cfg.path, cfg.autoflush);
        return cfg;
    }
};

template<class Backend>
struct factory_traits<sink::file_t<Backend, sink::rotator_t>> {
    typedef typename sink::file_t<Backend, sink::rotator_t>::config_type config_type;

    static config_type map_config(const boost::any& config) {
        config_type cfg;
        std::vector<boost::any> rotator;
        aux::vector_to(config, cfg.path, cfg.autoflush, rotator);
        aux::vector_to(rotator, cfg.rotator.size, cfg.rotator.count);
        return cfg;
    }
};

} // namespace blackhole
