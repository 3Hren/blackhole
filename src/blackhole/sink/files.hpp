#pragma once

#include "blackhole/attribute.hpp"
#include "blackhole/repository/factory/traits.hpp"
#include "blackhole/sink/files/backend.hpp"
#include "blackhole/sink/files/config.hpp"
#include "blackhole/sink/files/flusher.hpp"
#include "blackhole/sink/files/rotation.hpp"
#include "blackhole/sink/files/writer.hpp"
#include "blackhole/utils/unique.hpp"

namespace blackhole {

namespace sink {

template<class Backend, class Rotator, class = void>
class file_hander_t;

template<class Backend>
class file_hander_t<Backend, NoRotation, void> {
    Backend m_backend;
    files::writer_t<Backend> writer;
    files::flusher_t<Backend> flusher;
public:
    file_hander_t(const std::string& path, const files::config_t<NoRotation>& config) :
        m_backend(path),
        writer(m_backend),
        flusher(config.autoflush, m_backend)
    {}

    void handle(const std::string& message) {
        writer.write(message);
        flusher.flush();
    }

    const Backend& backend() {
        return m_backend;
    }
};

template<class Backend, class Rotator>
class file_hander_t<
    Backend,
    Rotator,
    typename std::enable_if<
        !std::is_same<Rotator, NoRotation>::value
    >::type>
{
    Backend m_backend;
    files::writer_t<Backend> writer;
    files::flusher_t<Backend> flusher;
    Rotator rotator;
public:
    file_hander_t(const std::string& path, const files::config_t<Rotator>& config) :
        m_backend(path),
        writer(m_backend),
        flusher(config.autoflush, m_backend),
        rotator(config.rotation, m_backend)
    {}

    void handle(const std::string& message) {
        writer.write(message);
        flusher.flush();
        if (BOOST_UNLIKELY(rotator.necessary(message))) {
            rotator.rotate();
        }
    }

    const Backend& backend() {
        return m_backend;
    }
};

template<class Backend = files::boost_backend_t, class Rotator = NoRotation>
class files_t {
    typedef file_hander_t<Backend, Rotator> file_handler_type;
    typedef std::map<std::string, std::shared_ptr<file_handler_type>> file_handlers_type;

    files::config_t<Rotator> config;
    file_handlers_type m_handlers;
public:
    typedef files::config_t<Rotator> config_type;

    static const char* name() {
        return "files";
    }

    files_t(const config_type& config) :
        config(config)
    {}

    //!@todo: second arg is temporary
    void consume(const std::string& message, const log::attributes_t& attributes = log::attributes_t()) {
        auto filename = make_filename(attributes);
        auto it = m_handlers.find(filename);
        if (it == m_handlers.end()) {
            it = m_handlers.insert(it, std::make_pair(filename, std::make_shared<file_handler_type>(filename, config)));
        }
        it->second->handle(message);
    }

    const file_handlers_type& handlers() {
        return m_handlers;
    }

    std::string make_filename(const log::attributes_t& attributes) const {
        auto it = config.path.begin();
        auto end = config.path.end();
        std::string filename;
        while (it != end) {
            if (*it == '%' && it + 1 != end && *(it + 1) == '(') {
                it += 2;

                std::string key;
                bool found = false;
                while (it != end) {
                    if (*it == ')' && it + 1 != end && *(it + 1) == 's') {
                        found = true;
                        it += 2;
                        break;
                    }
                    key.push_back(*it);
                    it++;
                }

                if (found) {
                    auto ait = attributes.find(key);
                    if (ait != attributes.end()) {
                        std::ostringstream stream;
                        stream << ait->second.value;
                        filename.append(stream.str());
                    } else {
                        filename.append(key);
                    }
                } else {
                    filename.append(key);
                }
            }

            if (it == end) {
                break;
            }
            filename.push_back(*it);
            it++;
        }
//        std::cout << "filename=" << filename << std::endl;
        return filename;
    }
};

} // namespace sink

template<class Backend, class Watcher>
struct unique_id_traits<sink::files_t<Backend, sink::rotator_t<Backend, Watcher>>> {
    typedef sink::rotator_t<Backend, Watcher> rotator_type;
    typedef sink::files_t<Backend, rotator_type> sink_type;

    typedef std::map<std::string, boost::any> variant_map_t;

    static std::string generate(const boost::any& config) {
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
