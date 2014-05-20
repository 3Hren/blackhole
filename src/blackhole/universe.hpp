#pragma once

#include <boost/thread/shared_mutex.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/utils/noncopyable.hpp"

namespace blackhole {

template<class T>
class singleton_mixin {
public:
    virtual ~singleton_mixin() {}

    static T& instance() {
        static T self;
        return self;
    }

protected:
    singleton_mixin() {}
};

class universe_storage_t : public singleton_mixin<universe_storage_t> {
    BLACKHOLE_DECLARE_NONCOPYABLE(universe_storage_t);
    friend class singleton_mixin<universe_storage_t>;

    typedef boost::shared_mutex mutex_type;

    log::attributes_t attributes;
    mutable mutex_type mutex;
public:
    void add(const std::string& key, const log::attribute_t& value) {
        boost::unique_lock<mutex_type> lock(mutex);
        attributes[key] = value;
    }

    void add(const log::attribute_pair_t& pair) {
        boost::unique_lock<mutex_type> lock(mutex);
        attributes.insert(pair);
    }

    log::attribute_t get(const std::string& key) const {
        boost::shared_lock<mutex_type> lock(mutex);
        return attributes.at(key);
    }

    log::attributes_t dump() const {
        boost::shared_lock<mutex_type> lock(mutex);
        return attributes;
    }

private:
    universe_storage_t() {}
};

} // namespace blackhole
