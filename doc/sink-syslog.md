## Writing to syslog

This example is all about syslog sink, which writes all incoming log messages to \*nix syslog. It's not so large as previous tutorial, but therefore contains one important feature I'd like to describe more in detail.

For a start, as usual, include all necessary header files and define severity enumeration:
{% highlight c++ %}
#include <blackhole/blackhole.hpp>
#include <blackhole/frontend/syslog.hpp>

enum class level {
    debug,
    warning,
    error
};
{% endhighlight %}

As you can see, as like in the previous example, additional header file is required - syslog frontend.

The fact that syslog has own severity level definition as it described in **RFC5424** and we need to properly map from user defined severity enumeration to the syslog's one. But there is no other way to pass additional parameter (current log event's severity) to the sink except implementing frontend's template specialization. That's why we need additional header file.

Okay, now we know that it is necessary to map severity levels, but how to do that? Blackhole provides you way to do that via implementing additional template specialization. The code will looks like this:
{% highlight c++ %}
namespace blackhole {

namespace sink {

template<>
struct priority_traits<level> {
    static priority_t map(level lvl) {
        switch (lvl) {
        case level::debug:
            return priority_t::debug;
        case level::warning:
            return priority_t::warning;
        case level::error:
            return priority_t::err;
        default:
            return priority_t::debug;
        }

        return priority_t::debug;
    }
};

} // namespace sink

} // namespace blackhole
{% endhighlight %}

We just implement `priority_traits` template specialization with single static method `map` that accepts single parameter - user-defined severity (`level` in our case) and returns syslog's one - `priority_t`. Mapping itself is a user's responsibility.

Remaining part of the example shouldn't be hard to understand if you are familiar with the previous example.

As always register necessary formatter and sink. Note that syslog sink requires user-defined severity enumeration symbol as template parameter. This information is needed for severity level mapping.
{% highlight c++ %}
repository_t::instance().configure<sink::syslog_t<level>, formatter::string_t>();
{% endhighlight %}

Further configuration also shouldn't cause difficulties.
{% highlight c++ %}
// Formatter is configured as usual, except we don't need anything than message.
formatter_config_t formatter("string");
formatter["pattern"] = "%(message)s";

// Syslog sink in its current implementation also hasn't large amount of options.
sink_config_t sink("syslog");
sink["identity"] = "test-application";

frontend_config_t frontend = { formatter, sink };
log_config_t config{ "root", { frontend } };

repository_t::instance().add_config(config);
{% endhighlight %}

Full example code is:
{% highlight c++ linenos %}
#include <blackhole/blackhole.hpp>
#include <blackhole/frontend/syslog.hpp>

//! This example demonstrates how to configure syslog sink and its features.
//!  - mapping from user-defined severity to the syslog's one.

enum class level {
    debug,
    warning,
    error
};

// To be able to properly map user-defined severity enumeration to the syslog's one
// we should implement special mapping trait that is called by library each time when
// mapping is required.
namespace blackhole {

namespace sink {

template<>
struct priority_traits<level> {
    static priority_t map(level lvl) {
        switch (lvl) {
        case level::debug:
            return priority_t::debug;
        case level::warning:
            return priority_t::warning;
        case level::error:
            return priority_t::err;
        default:
            return priority_t::debug;
        }

        return priority_t::debug;
    }
};

} // namespace sink

} // namespace blackhole

using namespace blackhole;

// Here we are going to configure our string/syslog frontend and to register it.
void init() {
    // As always register necessary formatter and sink. Note that syslog sink requires
    // user-defined severity enumeration symbol as template parameter.
    // This information is needed for severity level mapping.
    repository_t::instance().configure<sink::syslog_t<level>, formatter::string_t>();

    // Formatter is configured as usual, except we don't need anything than message.
    formatter_config_t formatter("string");
    formatter["pattern"] = "%(message)s";

    // Syslog sink in its current implementation also hasn't large amout of options.
    sink_config_t sink("syslog");
    sink["identity"] = "test-application";

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "root", { frontend } };

    repository_t::instance().add_config(config);
}

int main(int, char**) {
    init();
    verbose_logger_t<level> log = repository_t::instance().root<level>();

    BH_LOG(log, level::debug, "debug message");
    BH_LOG(log, level::warning, "warning message");
    BH_LOG(log, level::error, "error message");

    return 0;
}
{% endhighlight %}

After executing the next messages should be displayed in your syslog (depending on your syslog's configuration):
![Output after executing the example](images/docs/syslog-1.png)

*Note that debug message in my case was ignored by syslog itself, not by Blackhole.*