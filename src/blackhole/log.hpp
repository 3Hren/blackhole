#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/variant.hpp>

#include "utils/format.hpp"

namespace blackhole {

namespace log {

typedef boost::variant<
    std::time_t,
    std::uint8_t,
    std::uint64_t,
    std::int64_t,
    std::double_t,
    std::string
> attribute_value_t;

typedef std::pair<
    std::string,
    attribute_value_t
> attribute_pair_t;

typedef std::unordered_map<
    attribute_pair_t::first_type,
    attribute_pair_t::second_type
> attributes_t;

struct record_t {
    attributes_t attributes;

    bool valid() const {
        return !attributes.empty();
    }
};

} // namespace log

class error_t : public std::runtime_error {
public:
    template<typename... Args>
    error_t(const std::string& reason, Args&&... args) :
        std::runtime_error(utils::format(reason, std::forward<Args>(args)...))
    {}
};

namespace formatter {

class string_t {
    struct config_t {
        std::string pattern;
        std::vector<std::string> attribute_names;
    };

    const config_t m_config;
public:
    string_t(const std::string& pattern) :
        m_config(init(pattern))
    {
    }

    std::string
    format(const log::record_t& record) const {
        boost::format fmt(m_config.pattern);
        const log::attributes_t& attributes = record.attributes;
        const std::vector<std::string>& names = m_config.attribute_names;

        for (auto it = names.begin(); it != names.end(); ++it) {
            const std::string& name = *it;
            auto ait = attributes.find(name);
            if (ait == attributes.end()) {
                throw error_t("bad format string '%s' - key '%s' was not provided", m_config.pattern, name);
            }
            fmt % ait->second;
        }
        return fmt.str();
    }

private:
    static
    config_t
    init(const std::string& pattern) {
        auto current = pattern.begin();
        auto end = pattern.end();

        std::string fpattern;
        fpattern.reserve(pattern.length());
        std::vector<std::string> attribute_names;

        while(current != end) {
            if ((*current == '%') && (current + 1 != end) && (*(current + 1) == '(')) {
                fpattern.push_back('%');
                current += 2;
                std::string key;
                while (current != end) {
                    if ((*current == ')') && (current + 1 != end) && (*(current + 1) == 's')) {
                        break;
                    } else {
                        key.push_back(*current);
                    }
                    current++;
                }
                attribute_names.push_back(key);
            } else {
                fpattern.push_back(*current);
            }
            current++;
        }

        return { fpattern, attribute_names };
    }
};

} // namespace formatter;

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

}

} // namespace blackhole
