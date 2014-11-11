#pragma once

#include "logger.hpp"
#include "logger/wrapper.hpp"

namespace blackhole {

class scoped_attributes_t : public scoped_attributes_concept_t {
    // Attributes provided by this guard.
    mutable attribute::set_t m_guard_attributes;
    // Merged attributes are provided by this guard and all the parent guards.
    // This value is computed lazily.
    mutable attribute::set_t m_merged_attributes;

public:
    scoped_attributes_t(logger_base_t& logger, attribute::set_t attributes) :
        scoped_attributes_concept_t(logger),
        m_guard_attributes(std::move(attributes))
    {}

    template<class Wrapper>
    scoped_attributes_t(
        Wrapper& wrapper,
        attribute::set_t attributes,
        typename std::enable_if<
            !std::is_base_of<logger_base_t, Wrapper>::value
        >::type* = 0
    ) :
        scoped_attributes_concept_t(wrapper.log()),
        m_guard_attributes(std::move(attributes))
    {}

    virtual
    const attribute::set_t&
    attributes() const {
        if (m_merged_attributes.empty()) {
            m_merged_attributes = std::move(m_guard_attributes);
            if (has_parent()) {
                const auto& parent_attributes = parent().attributes();
                std::copy(parent_attributes.begin(), parent_attributes.end(), std::back_inserter(m_merged_attributes));
            }
        }
        return m_merged_attributes;
    }
};

} // namespace blackhole
