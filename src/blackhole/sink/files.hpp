#pragma once

#include "blackhole/factory.hpp"
#include "blackhole/sink/files/backend.hpp"
#include "blackhole/sink/files/config.hpp"
#include "blackhole/sink/files/flusher.hpp"
#include "blackhole/sink/files/rotation.hpp"
#include "blackhole/sink/files/writer.hpp"

namespace blackhole {

namespace sink {

template<class Backend = files::boost_backend_t, class Rotator = NoRotation, typename = void>
class files_t;

template<class Backend>
class files_t<Backend, NoRotation, void> {
    Backend m_backend;
    files::writer_t<Backend> m_writer;
    files::flusher_t<Backend> m_flusher;
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
    files::writer_t<Backend> m_writer;
    files::flusher_t<Backend> m_flusher;
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
    typedef sink::rotator_t<Backend, Watcher> rotator_type;
    typedef sink::files_t<Backend, rotator_type> sink_type;

    typedef std::map<std::string, boost::any> variant_map_t;

    static std::string extract(const boost::any& config) {
        const variant_map_t& cfg = boost::any_cast<variant_map_t>(config);

        auto rotation_it = cfg.find("rotation");
        if (rotation_it != cfg.end()) {
            const variant_map_t& rotation = boost::any_cast<variant_map_t>(rotation_it->second);

            const bool has_size_watcher = rotation.find("size") != rotation.end();
            const bool has_datetime_watcher = rotation.find("period") != rotation.end();

            if (has_size_watcher && has_datetime_watcher) {
                throw blackhole::error_t("set watcher is not implemented yet");
            } else if (has_size_watcher) {
                return utils::format("%s/%s/%s", sink_type::name(), rotator_type::name(), "size");
            } else if (has_datetime_watcher) {
                return utils::format("%s/%s/%s", sink_type::name(), rotator_type::name(), "datetime");
            }

            throw blackhole::error_t("rotation section not properly configures: no watcher settings found");
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

template<class Backend, class Watcher>
struct config_traits<sink::files_t<Backend, sink::rotator_t<Backend, Watcher>>> {
    typedef sink::rotator_t<Backend, Watcher> rotator_type;
    typedef sink::files_t<Backend, rotator_type> sink_type;

    static std::string name() {
        return utils::format("%s/%s/%s", sink_type::name(), rotator_type::name(), Watcher::name());
    }
};

namespace aux {

template<class Backend, class Rotator>
struct filler<sink::files_t<Backend, Rotator>> {
    template<class Extractor, class Config>
    static void extract_to(const Extractor& ex, Config& config) {
        ex["path"].to(config.path);
        ex["autoflush"].to(config.autoflush);
    }
};

template<class Backend, class Watcher>
struct filler<sink::rotator_t<Backend, Watcher>> {
    template<class Extractor, class Config>
    static void extract_to(const Extractor& ex, Config& config) {
        ex["rotation"]["pattern"].to(config.rotation.pattern);
        ex["rotation"]["backups"].to(config.rotation.backups);
    }
};

} // namespace aux

template<class Backend>
struct factory_traits<sink::files_t<Backend>> {
    typedef sink::files_t<Backend> sink_type;
    typedef typename sink_type::config_type config_type;

    static void map_config(const aux::extractor<sink_type>& ex, config_type& config) {
        aux::filler<sink_type>::extract_to(ex, config);
    }
};

template<class Backend>
struct factory_traits<sink::files_t<Backend, sink::rotator_t<Backend, sink::rotation::watcher::size_t>>> {
    typedef sink::rotation::watcher::size_t watcher_type;
    typedef sink::rotator_t<Backend, watcher_type> rotator_type;
    typedef sink::files_t<Backend, rotator_type> sink_type;
    typedef typename sink_type::config_type config_type;

    static void map_config(const aux::extractor<sink_type>& ex, config_type& config) {
        aux::filler<sink_type>::extract_to(ex, config);
        aux::filler<rotator_type>::extract_to(ex, config);
        ex["rotation"]["size"].to(config.rotation.watcher.size);
    }
};

template<class Backend>
struct factory_traits<sink::files_t<Backend, sink::rotator_t<Backend, sink::rotation::watcher::datetime_t<>>>> {
    typedef sink::rotation::watcher::datetime_t<> watcher_type;
    typedef sink::rotator_t<Backend, watcher_type> rotator_type;
    typedef sink::files_t<Backend, rotator_type> sink_type;
    typedef typename sink_type::config_type config_type;

    static void map_config(const aux::extractor<sink_type>& ex, config_type& config) {
        aux::filler<sink_type>::extract_to(ex, config);
        aux::filler<rotator_type>::extract_to(ex, config);
        ex["rotation"]["period"].to(config.rotation.watcher.period);
    }
};

} // namespace blackhole
