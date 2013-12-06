#pragma once

#include <memory>
#include <vector>

#include "attribute.hpp"
#include "filter.hpp"
#include "frontend.hpp"
#include "keyword.hpp"
#include "keyword/severity.hpp"
#include "keyword/timestamp.hpp"

namespace blackhole {

class logger_base_t {
    bool m_enabled;

protected:
    filter_t m_filter;
    std::vector<std::unique_ptr<base_frontend_t>> m_frontends;
    log::attributes_t m_global_attributes;

public:
    logger_base_t() :
        m_enabled(true),
        m_filter(default_filter_t::instance())
    {}

    bool enabled() const {
        return m_enabled;
    }

    void enable() {
        m_enabled = true;
    }

    void disable() {
        m_enabled = false;
    }

    void set_filter(filter_t&& filter) {
        m_filter = filter;
    }

    void add_attribute(const log::attribute_pair_t& attr) {
        m_global_attributes.insert(attr);
    }

    void add_frontend(std::unique_ptr<base_frontend_t> frontend) {
        m_frontends.push_back(std::move(frontend));
    }

    log::record_t open_record() const {
        return open_record(log::attributes_t());
    }

    log::record_t open_record(log::attribute_pair_t&& local_attribute) {
        return open_record(log::attributes_t({ std::move(local_attribute) }));
    }

    log::record_t open_record(log::attributes_t&& local_attributes) const {
        if (enabled() && !m_frontends.empty()) {
            log::attributes_t attributes = merge({
                // universe_attributes              // Program global.
                // thread_attributes                // Thread local.
                m_global_attributes,                // Logger object specific.
                std::move(get_scoped_attributes()), // Depending on event scope, e.g. timestamp.
                std::move(local_attributes)         // Any user attributes.
            });

            if (m_filter(attributes)) {
                log::record_t record;
                record.attributes = std::move(attributes);
                return record;
            }
        }

        return log::record_t();
    }

    void push(log::record_t&& record) const {
        for (auto it = m_frontends.begin(); it != m_frontends.end(); ++it) {
            const std::unique_ptr<base_frontend_t>& frontend = *it;
            frontend->handle(record);
        }
    }

private:
    log::attributes_t get_scoped_attributes() const {
        //!@todo: Declare attribute type in DECLARE_KEYWORD or explicitly.
        log::attributes_t attributes = {
            keyword::timestamp_id() = std::time(nullptr)
        };
        return attributes;
    }
};

template<typename Level>
class verbose_logger_t : public logger_base_t {
    typedef typename std::underlying_type<Level>::type level_type;

public:
    log::record_t open_record(Level level) const {
        return logger_base_t::open_record({ keyword::severity<Level>() = level });
    }
};

} // namespace blackhole
