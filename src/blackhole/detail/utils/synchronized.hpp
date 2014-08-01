/*
 * Copyright (c) 2011-2014 Andrey Sibiryov <me@kobology.ru>
 */

#pragma once

#include <mutex>

namespace blackhole {

namespace aux {

namespace utils {

template<class T, class Lockable = std::mutex>
struct locked_ptr {
    typedef T        value_type;
    typedef Lockable mutex_type;

    locked_ptr(value_type& value, mutex_type& mutex): value(value), guard(mutex) { }
    locked_ptr(locked_ptr&& o): value(o.value), guard(std::move(o.guard)) { }

    T* operator->() { return &value; }
    T& operator* () { return  value; }

private:
    value_type& value;
    std::unique_lock<mutex_type> guard;
};

template<class T, class Lockable>
struct locked_ptr<const T, Lockable> {
    typedef T        value_type;
    typedef Lockable mutex_type;

    locked_ptr(const value_type& value, mutex_type& mutex): value(value), guard(mutex) {}
    locked_ptr(locked_ptr&& o): value(o.value), guard(std::move(o.guard)) {}

    const T* operator->() const { return &value; }
    const T& operator* () const { return  value; }

private:
    const value_type& value;
    std::unique_lock<mutex_type> guard;
};

template<class T, class Lockable = std::mutex>
struct synchronized {
    typedef T        value_type;
    typedef Lockable mutex_type;

    synchronized(): m_value() { }
    synchronized(const value_type& value): m_value(value) { }
    synchronized(value_type&& value): m_value(std::move(value)) { }

    auto
    value() -> value_type& {
        return m_value;
    }

    auto
    value() const -> const value_type& {
        return m_value;
    }

    typedef locked_ptr<T, Lockable> ptr_type;
    typedef locked_ptr<const T, Lockable> const_ptr_type;

    auto
    synchronize() -> ptr_type {
        return ptr_type(m_value, m_mutex);
    }

    auto
    synchronize() const -> const_ptr_type {
        return const_ptr_type(m_value, m_mutex);
    }

    auto
    operator->() -> ptr_type {
        return synchronize();
    }

    auto
    operator->() const -> const_ptr_type {
        return synchronize();
    }

private:
    value_type m_value;
    mutable mutex_type m_mutex;
};

}

}

}
