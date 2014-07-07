#pragma once

#include <mutex>

namespace blackhole {

class logger_base_t;

template<typename Level>
class verbose_logger_t;

template<class Logger>
class wrapper_t;

template<class T, class Mutex = std::mutex>
class synchronized;

} // namespace blackhole
