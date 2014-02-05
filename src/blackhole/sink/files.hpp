#pragma once

#include "blackhole/error.hpp"
#include "blackhole/factory.hpp"
#include "blackhole/sink/files/backend.hpp"
#include "blackhole/sink/files/rotation.hpp"

namespace blackhole {

namespace sink {

namespace files {

template<class Rotator = NoRotation>
struct config_t {
    std::string path;
    bool autoflush;

    config_t(const std::string& path = "/dev/stdout", bool autoflush = true) :
        path(path),
        autoflush(autoflush)
    {}
};

template<class Backend, class Watcher, class Timer>
struct config_t<rotator_t<Backend, Watcher, Timer>> {
    std::string path;
    bool autoflush;
    rotation::config_t<Watcher> rotation;

    config_t(const std::string& path = "/dev/stdout",
             bool autoflush = true,
             const rotation::config_t<Watcher>& rotation = rotation::config_t<Watcher>()) :
        path(path),
        autoflush(autoflush),
        rotation(rotation)
    {}
};

} // namespace files

template<class Backend>
class writer_t {
    Backend& backend;
public:
    writer_t(Backend& backend) :
        backend(backend)
    {}

    void write(const std::string& message) {
        if (!backend.opened()) {
            if (!backend.open()) {
                throw error_t("failed to open file '%s' for writing", backend.path());
            }
        }
        backend.write(message);
    }
};

template<class Backend>
class flusher_t {
    bool autoflush;
    Backend& backend;
public:
    flusher_t(bool autoflush, Backend& backend) :
        autoflush(autoflush),
        backend(backend)
    {}

    void flush() {
        if (autoflush) {
            backend.flush();
        }
    }
};

template<class Backend = files::boost_backend_t, class Rotator = NoRotation, typename = void>
class files_t;

template<class Backend>
class files_t<Backend, NoRotation, void> {
    Backend m_backend;
    writer_t<Backend> m_writer;
    flusher_t<Backend> m_flusher;
public:
    typedef files::config_t<NoRotation> config_type;

    static const char* name() {
        return "files";
    }

    files_t(const config_type& config) :
        m_backend(config.path),
        m_writer(m_backend),
        m_flusher(config.autoflush, m_backend)
    {}

    void consume(const std::string& message) {
        m_writer.write(message);
        m_flusher.flush();
    }

    Backend& backend() {
        return m_backend;
    }
};

template<class Backend, class Rotator>
class files_t<
    Backend,
    Rotator,
    typename std::enable_if<
        !std::is_same<Rotator, NoRotation>::value
    >::type>
{
    Backend m_backend;
    writer_t<Backend> m_writer;
    flusher_t<Backend> m_flusher;
    Rotator m_rotator;
public:
    typedef files::config_t<Rotator> config_type;

    static const char* name() {
        return "files";
    }

    files_t(const config_type& config) :
        m_backend(config.path),
        m_writer(m_backend),
        m_flusher(config.autoflush, m_backend),
        m_rotator(config.rotation, m_backend)
    {}

    void consume(const std::string& message) {
        m_writer.write(message);
        m_flusher.flush();
        if (m_rotator.necessary(message)) {
            m_rotator.rotate();
        }
    }

    Backend& backend() {
        return m_backend;
    }
};

} // namespace sink

namespace generator {

template<class Backend, class Watcher>
struct id<sink::files_t<Backend, sink::rotator_t<Backend, Watcher>>> {
    static std::string extract(const boost::any& config) {
        typedef sink::rotator_t<Backend, Watcher> rotator_type;
        typedef sink::files_t<Backend, rotator_type> sink_type;

        std::map<std::string, boost::any> cfg;
        aux::any_to(config, cfg);

        if (cfg.find("rotation") != cfg.end()) {
            return utils::format("%s/%s", sink_type::name(), rotator_type::name());
        }

        return sink_type::name();
    }
};

} // namespace generator

template<class Backend>
struct config_traits<sink::files_t<Backend, sink::NoRotation>> {
    static std::string name() {
        return sink::files_t<Backend, sink::NoRotation>::name();
    }
};

template<class Backend, class Rotator>
struct config_traits<sink::files_t<Backend, Rotator>> {
    static std::string name() {
        return utils::format("%s/%s", sink::files_t<Backend, Rotator>::name(), Rotator::name());
    }
};

template<class Backend>
struct factory_traits<sink::files_t<Backend>> {
    typedef sink::files_t<Backend> sink_type;
    typedef typename sink_type::config_type config_type;

    static config_type map_config(const boost::any& config) {
        config_type cfg;
        aux::extractor<sink::files_t<Backend>> ex(config);
        ex["path"].to(cfg.path);
        ex["autoflush"].to(cfg.autoflush);
        return cfg;
    }
};

template<class Backend>
struct factory_traits<sink::files_t<Backend, sink::rotator_t<Backend, sink::rotation::watcher::size_t>>> {
    typedef typename sink::files_t<Backend, sink::rotator_t<Backend, sink::rotation::watcher::size_t>> sink_type;
    typedef typename sink_type::config_type config_type;

    static config_type map_config(const boost::any& config) {
        config_type cfg;
        aux::extractor<sink_type> ex(config);
        ex["path"].to(cfg.path);
        ex["autoflush"].to(cfg.autoflush);
        ex["rotation"]["pattern"].to(cfg.rotation.pattern);
        ex["rotation"]["backups"].to(cfg.rotation.backups);
        ex["rotation"]["size"].to(cfg.rotation.watcher.size);
        return cfg;
    }
};

} // namespace blackhole
