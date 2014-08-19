#pragma once

#include <boost/thread/shared_mutex.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/detail/config/noncopyable.hpp"

//!@todo: Drop this shit.
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

    attribute::set_t attributes;
    mutable mutex_type mutex;
public:
    void add(const std::string& key, const attribute_t& value) {
        boost::unique_lock<mutex_type> lock(mutex);
        attributes[key] = value;
    }

    void add(const attribute::pair_t& pair) {
        boost::unique_lock<mutex_type> lock(mutex);
        attributes.insert(pair);
    }

    attribute_t get(const std::string& key) const {
        boost::shared_lock<mutex_type> lock(mutex);
        return attributes.at(key);
    }

    attribute::set_t dump() const {
        boost::shared_lock<mutex_type> lock(mutex);
        return attributes;
    }

private:
    universe_storage_t() {}
};

} // namespace blackhole
