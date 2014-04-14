#pragma once

#include <memory>
#include <vector>

#include "attribute.hpp"
#include "common.hpp"
#include "error/handler.hpp"
#include "filter.hpp"
#include "frontend.hpp"
#include "keyword.hpp"
#include "keyword/message.hpp"
#include "keyword/severity.hpp"
#include "keyword/timestamp.hpp"
#include "keyword/thread.hpp"
#include "universe.hpp"
#include "blackhole/utils/noexcept.hpp"
#include "utils/unique.hpp"

namespace blackhole {

class logger_base_t {
    bool m_enabled;

protected:
    filter_t m_filter;
    log::exception_handler_t m_exception_handler;

    std::vector<std::unique_ptr<base_frontend_t>> m_frontends;

    log::attributes_t m_global_attributes;

public:
    logger_base_t() :
        m_enabled(true),
        m_filter(default_filter_t::instance()),
        m_exception_handler(log::default_exception_handler_t())
    {}

    // Blaming GCC 4.4 - it needs explicit move constructor definition,
    // because it cannot define default move constructor for derived class.
    logger_base_t(logger_base_t&& other) BLACKHOLE_NOEXCEPT {
        *this = std::move(other);
    }

    logger_base_t& operator=(logger_base_t&& other) BLACKHOLE_NOEXCEPT {
        swap(*this, other);
        return *this;
    }

    friend void swap(logger_base_t& lhs, logger_base_t& rhs) BLACKHOLE_NOEXCEPT {
        using std::swap;
        std::swap(lhs.m_enabled, rhs.m_enabled);
        std::swap(lhs.m_filter, rhs.m_filter);
        std::swap(lhs.m_exception_handler, rhs.m_exception_handler);
        std::swap(lhs.m_frontends, rhs.m_frontends);
        std::swap(lhs.m_global_attributes, rhs.m_global_attributes);
    }

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
        m_filter = std::move(filter);
    }

    void add_attribute(const log::attribute_pair_t& attr) {
        m_global_attributes.insert(attr);
    }

    void add_frontend(std::unique_ptr<base_frontend_t> frontend) {
        m_frontends.push_back(std::move(frontend));
    }

    void set_exception_handler(log::exception_handler_t&& handler) {
        m_exception_handler = std::move(handler);
    }

    log::record_t open_record() const {
        return open_record(log::attributes_t());
    }

    log::record_t open_record(log::attribute_pair_t&& local_attribute) const {
        return open_record(log::attributes_t({ std::move(local_attribute) }));
    }

    log::record_t open_record(log::attributes_t&& local_attributes) const {
        if (enabled() && !m_frontends.empty()) {
            log::attributes_t attributes = merge({
                universe_storage_t::instance().dump(),  // Program global.
                get_thread_attributes(),                // Thread local.
                m_global_attributes,                    // Logger object specific.
                get_event_attributes(),                 // Event specific, e.g. timestamp.
                std::move(local_attributes)             // Any user attributes.
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
            try {
                const std::unique_ptr<base_frontend_t>& frontend = *it;
                frontend->handle(record);
            } catch (...) {
                m_exception_handler();
            }
        }
    }

private:
    log::attributes_t get_event_attributes() const {
        timeval tv;
        gettimeofday(&tv, nullptr);
        log::attributes_t attributes = {
            keyword::timestamp() = tv
        };
        return attributes;
    }

    log::attributes_t get_thread_attributes() const {
        log::attributes_t attributes = {
            keyword::tid() = this_thread::get_id<std::string>()
        };
        return attributes;
    }
};

template<typename Level>
class verbose_logger_t : public logger_base_t {
    typedef Level level_type;

public:
    verbose_logger_t() :
        logger_base_t()
    {}

    // GCC 4.4 doesn't create default copy/move constructor for derived classes. It's a bug.
    verbose_logger_t(verbose_logger_t&& other) BLACKHOLE_NOEXCEPT :
        logger_base_t(std::move(other))
    {}

    verbose_logger_t& operator=(verbose_logger_t&& other) BLACKHOLE_NOEXCEPT {
        logger_base_t::operator=(std::move(other));
        return *this;
    }

    log::record_t open_record(Level level) const {
        log::attributes_t attributes = { keyword::severity<Level>() = level };
        return logger_base_t::open_record(std::move(attributes));
    }
};

} // namespace blackhole
