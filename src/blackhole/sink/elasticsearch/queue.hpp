#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

namespace blackhole {

namespace sink {

namespace bulk {

template<typename T>
class queue_t {
public:
    typedef T value_type;
    typedef std::vector<value_type> bulk_type;
    typedef std::function<void(bulk_type&&)> callback_type;

private:
    const std::uint16_t bulk;
    callback_type callback;

    std::queue<T> queue;
    mutable std::mutex mutex;

public:
    queue_t(std::uint16_t bulk, callback_type callback) :
        bulk(bulk),
        callback(callback)
    {}

    void push(T&& value) {
        std::unique_lock<std::mutex> lock(mutex);

        queue.push(std::move(value));
        if (queue.size() >= bulk) {
            auto result = dump(lock);
            lock.unlock();
            callback(std::move(result));
        }
    }

    std::vector<std::string> dump() {
        std::lock_guard<std::mutex> lock(mutex);
        return dump(lock);
    }

private:
    template<class Lock>
    std::vector<std::string> dump(Lock&) {
        std::vector<std::string> result;
        result.reserve(bulk);
        for (uint i = 0; i < bulk && !queue.empty(); ++i) {
            result.push_back(std::move(queue.front()));
            queue.pop();
        }
        return result;
    }
};

} // namespace bulk

} // namespace sink

} // namespace blackhole
