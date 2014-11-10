#pragma once

#include <mutex>

/*!
 * Namespace indents were made intentionally to improve the readability.
 */
namespace blackhole {
    namespace repository {
        namespace config {
            template<class T>
            struct transformer_t;

            template<class To>
            class parser_t;
        } // namespace config
    } // namespace repository

    namespace attribute {
        class set_view_t;
    } // namespace attribute

    class base_frontend_t;
    class frontend_factory_t;

    class formatter_config_t;
    class sink_config_t;

    class record_t;

    class logger_base_t;

    template<typename Level>
    class verbose_logger_t;

    template<class Logger, class = void>
    class wrapper_t;

    template<class T, class Mutex = std::mutex>
    class synchronized;
} // namespace blackhole
