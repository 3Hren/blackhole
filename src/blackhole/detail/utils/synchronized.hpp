/*
 * Copyright (c) 2011-2014 Andrey Sibiryov <me@kobology.ru>
 * Improved by Evgeny Safronov <division494@gmail.com>.
 */

#pragma once

#include <mutex>

namespace blackhole {

namespace aux {

namespace utils {

template<class T, class Mutex = std::mutex, class Lock = std::lock_guard<Mutex>>
struct locked_ptr {
    typedef T     value_type;
    typedef Mutex mutex_type;
    typedef Lock  lock_type;

    locked_ptr(value_type& value, mutex_type& mutex) : value(value), lock(mutex) {}
    locked_ptr(locked_ptr&& o) : value(o.value), lock(std::move(o.lock)) {}

    T* operator->() { return &value; }
    T& operator* () { return  value; }

private:
    value_type& value;
    lock_type lock;
};

template<class T, class Mutex, class Lock>
struct locked_ptr<const T, Mutex, Lock> {
    typedef T     value_type;
    typedef Mutex mutex_type;
    typedef Lock  lock_type;

    locked_ptr(const value_type& value, mutex_type& mutex) : value(value), lock(mutex) {}
    locked_ptr(locked_ptr&& o) : value(o.value), lock(std::move(o.lock)) {}

    const T* operator->() const { return &value; }
    const T& operator* () const { return  value; }

private:
    const value_type& value;
    lock_type lock;
};

template<
    class T,
    class Mutex = std::mutex,
    class ReaderLock = std::lock_guard<Mutex>,
    class WriterLock = std::lock_guard<Mutex>
>
struct synchronized {
    typedef T          value_type;
    typedef Mutex      mutex_type;
    typedef ReaderLock reader_lock_type;
    typedef WriterLock writer_lock_type;

    synchronized() : value_() { }
    synchronized(const value_type& value) : value_(value) {}
    synchronized(value_type&& value) : value_(std::move(value)) {}

    auto
    value() -> value_type& {
        return value_;
    }

    auto
    value() const -> const value_type& {
        return value_;
    }

    typedef locked_ptr<T, Mutex, WriterLock>       ptr_type;
    typedef locked_ptr<const T, Mutex, ReaderLock> const_ptr_type;

    auto
    synchronize() -> ptr_type {
        return ptr_type(value_, mutex);
    }

    auto
    synchronize() const -> const_ptr_type {
        return const_ptr_type(value_, mutex);
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
    value_type value_;
    mutable mutex_type mutex;
};

} // namespace utils

} // namespace aux

} // namespace blackhole
