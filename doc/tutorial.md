# Tutorial

  * [The simpliest example](#the-simpliest-example)
  * [Log into the file with rotation](#log-into-the-file-with-rotation)

## The simpliest example
Writing to log with the default settings of Blackhole.

By default, Blackhole initializes and configures logger `root` with single frontend, configured like this:

* Formatter: string with pattern `[%(timestamp)s] [%(severity)s]: %(message)s`;
* Sink: `stream` with `stdout` output.

The complete example code:

```
#include <blackhole/blackhole.hpp>

using namespace blackhole;

enum class level {
    debug,
    info,
    warning,
    error
};


void init(){
    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s] [%(severity)s]: %(message)s";

    sink_config_t sink("stream");
    sink["output"] = "stdout";

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "stdout_log", { frontend } }; //"stdout_log" is a name of our config

    repository_t::instance().add_config(config);
}


int main(int, char**) {
    
    init();

    //Do you remember what "stdout_log" it is? (Check the "init" function above)
    verbose_logger_t<level> log = repository_t::instance().create<level>("stdout_log"); 

    BH_LOG(log, level::debug,   "[%d] %s - done", 0, "debug");
    BH_LOG(log, level::info,    "[%d] %s - done", 1, "info");
    BH_LOG(log, level::warning, "[%d] %s - done", 2, "warning");
    BH_LOG(log, level::error,   "[%d] %s - done", 3, "error");
    return 0;
}
```

In Ubuntu you can compile it with the command `g++ simple_example.cpp -osimple_example -std=c++0x -lboost_thread-mt`.

`enum class level` is a declaration of severity levels for our logger. Blackhole supports both loggers with severity separation and without it. But, seriously, what is the logger without separated severity levels?

All logger objects in Blackhole are got from `repository_t` object. It is a singleton that can store all possible frontends configurations. We need to create configuration (formatter and sink pair) and add it to the `repository_t` object as in `void init()` function. After that you can create logger object as follows:

```
verbose_logger_t<level> log = repository_t::instance().create<level>("stdout_log"); //"stdout_log" is a name of previousely created config
```

Having the logger object we can pass in to the main logging macro which has the next signature:

```
BH_LOG(verbose_logger_t<Severity> logger, Severity severity, const char* message, Args...);
```

`Args...` - are the message's placeholder arguments for pretty formatting like `printf` style.

As a result of our example, we see the following in the console:

```
    [1396259922.290809] [0]: [0] debug - done
    [1396259922.306820] [1]: [1] info - done
    [1396259922.306926] [2]: [2] warning - done
    [1396259922.307020] [3]: [3] error - done
```

Using of other sinks and formatters may be specific. Before use of them read the corresponding parts of documentation:
  * [Sinks](sinks.md);
  * [Fomatters](formatters.md).

The most common words on Blackhole usage you can find in ["Common usage"](common-usage.md) article. Also there you can find some information on opaque methods of logger usage.

Now we recommend you to examine our next example on logging to the files.

## Log into the files with rotation

This example is about using file logger.
We consider how to register complex file sink with rotation support and how to configure it to have multiple files which names will be chosen depending on log event attributes.

The complete example code:

```(c++)
#include <blackhole/blackhole.hpp>
#include <blackhole/frontend/files.hpp>

using namespace blackhole;

enum class level {
    debug,
    error
};

void init(){
    repository_t::instance().configure<
        sink::files_t<
            sink::files::boost_backend_t,
            sink::rotator_t<
                sink::files::boost_backend_t,
                sink::rotation::watcher::size_t
            >
        >,
        formatter::string_t
    >();


    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s] - <%(severity)s>:%(message)s:Issue - %(issue)s";

    sink_config_t sink("files");
    sink["path"] = "./logs/blackhole-%(host)s.log";
    sink["autoflush"] = true;
    sink["rotation"]["pattern"] = "%(filename)s.%N";
    sink["rotation"]["backups"] = std::uint16_t(10);
    sink["rotation"]["size"] = std::uint64_t(4*1024);

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "rotating_file_log", { frontend } };

    repository_t::instance().add_config(config);
}

int main(int, char**) {

    init();
    
    verbose_logger_t<level> log = repository_t::instance().create<level>("rotating_file_log");

    for (int i = 0; i < 64; ++i) {
        BH_LOG(log, level::debug, "debug event")("host", "127.0.0.1", "issue", "The Ultimate Question of Life, the Universe, and Everything.");
        BH_LOG(log, level::error, "error event")("host", "localhost", "issue", "To be, or not to be: that is the question.");
    }

    return 0;
}

```

In Ubuntu you can compile it with the command `g++ rotating_files.cpp -ofiles -std=c++0x -lboost_thread-mt -lboost_filesystem -lboost_system`.

The first difference you can see is a new header:

```
#include <blackhole/sink/files.hpp>
```

The next difference contained in the next piece of code:

```
repository_t::instance().configure<
        sink::files_t<
            sink::files::boost_backend_t,
            sink::rotator_t<
                sink::files::boost_backend_t,
                sink::rotation::watcher::size_t
            >
        >,
        formatter::string_t
    >();
```

This is a registration code of `files`-sink. It may look frustrating and smells like dark template magic. This is an architectural solution that is discussed in "Architecture" part of documentation.

Required logger's object can be properly created only after registering frontend with corresponding configuration. Without registration an exception will be thrown.

Formatter configuration has no any surprises, just an `%(issue)s` attribute that is not supported with our `BH_LOG` macro from the starting point of view. We will discuss it a bit later.

Configuration of `files`-sink with rotation support looks more intresting then in the first example:

```(c++)
sink_config_t sink("files");
sink["path"] = "./logs/blackhole-%(host)s.log";
sink["autoflush"] = true;                           //Dump messages to the file immediately
sink["rotation"]["pattern"] = "%(filename)s.%N";    //Pattern for backup file name
sink["rotation"]["backups"] = std::uint16_t(10);    //Number of backups
sink["rotation"]["size"] = std::uint64_t(4*1024);   //Size of log file
```

Rotation works in the next manner. Blackhole writes logs into the log file (`path`) until it exceeds `size`. At that moment log file is renamed according to `pattern` and a new log file is started. Number of renamed files equals to `backups`. When last `backups` number exceeded cycle is repeated.

`%(host)s` placeholder in an attribute of log event, but the `%(filename)s.%N` is not. `%(filename)s.%N` pattern leads to creation of backup filenames from the active log file substituting `%N` number of backup (from 1 to `backups`).

More about `files` sink you can find in [detailed description](sink-files.md).


*Note that also date-time rotation can be specified, e.g. each day or each hour, but not now. More detailed it will be discussed in reference documentation.*

Next steps should be already familiar for people who passes previous tutorial. We just create frontend and logger configuration objects and push the last one into the repository.

```
frontend_config_t frontend = { formatter, sink };
log_config_t config{ "rotating_file_log", { frontend } };

repository_t::instance().add_config(config);
```

Only remaining to record a message to log in the `main`-function:

```
BH_LOG(log, level::error, "error event")("host", "localhost", "issue", "To be, or not to be: that is the question.");
```

What do you think about the second braces near our macro?

Welcome to the **dynamic attributes** setting into the log event! Every log event can transport any number of additional attributes, which will participate in all subsequent operations associated with it (look at the architecture diagram, if you forgot). `%(hosts)s` and `%(issue)s` placeholders in code contain the names of this attributes which should be passed to `BH_LOG` in the second parentesses. You can pass this attributes in different manner, read the ["Common usage"](common-usage.md) artricle.

====================================================================================================================

This tutorial describes how to configure formatters and sinks more deeply. Also it will be shown how to use logger API directly and what's hidden under that macro's hood.

Let's start with the same steps we are done in the previous tutorial - include main header file and define severity enumeration.

```
#include <blackhole/blackhole.hpp>

enum class level {
    debug
};

using namespace blackhole;
```

For this example we don't need multiple severity levels, so the single debug level is enough. But here the differences comes. In previous example we didn't have control for string formatter as like as stream sink. Now we want to control everything.

### Need more control

Let's specify own pattern for every log message, say, something like this:

`[%(timestamp)s] <%(severity)s>:: %(message)s`

At this point let's talk about string formatter pattern syntax. Consider pattern specified above. It has printf-like syntax with attribute names between `%(` and `)s` symbols and behaves just like you imagine it. If parser meets that attribute name, it will substitute real attribute value instead of it. For example if the log record attributes are:

    timestamp = 1396259922.000000
    severity  = 1
    message   = an intresting log message

formatter will produce the next string: `[1396259922.000000] <1>:: an intresting log message`.

To configure formatter in code we just create object of `formatter_config_t` class which has overloaded `operator[]` and provides `std::map` like interface giving ability to make hierarchical configuration.

```
formatter_config_t formatter("string");
formatter["pattern"] = "[%(timestamp)s] <%(severity)s>:: %(message)s";
```

*Note that at this moment string formatter has no other options.*

*Why not using strongly-typed configuration objects? Mainly because one of the requirements to logger was to be able to initialize it through configuration file, string or whatever else. Internally that weakly and unsafe configuration objects are mapped to its actual formatter's or sink's settings. Intentionally that mappers must be written by developer who has written that formatter/sink. It opens possibility to have full control over which settings must be specified everything and which of them can be passed or initialized by default. Developer also controls error handling. If some required options are not provided or there is typo mistake an exception will be thrown at moment of creation logger object via repository.*

Stream sink can be configured the same way. Say, we want to write our messages into `stdout`. Stream sinks has only one option - output type, which can be either `stdout` or `stderr` at this moment. The code is:

```
sink_config_t sink("stream");
sink["output"] = "stdout";
```

After customizing our future formatter and sink we must combine it into frontend and pass it (at last) to the logger config, not forgetting to specify logger's name. Later exactly by this name we can create new logger object with its configuration. All loggers config objects must be registered into the repository, our old friend, which we was discussed not so far. They will be there until program finished or until another logger config with the same name will be added. These steps in the code looks like:

```
frontend_config_t frontend = { formatter, sink };
log_config_t config{ "root", { frontend } };

repository_t::instance().add_config(config);
```

After that we have fully configured logger repository, which awaits until we create logger object itself (by name or by *special* name **root**, which doesn't means anything except cool The Mainest Logger name).

Usage pattern is the same as in previous example. We just create root logger object via repository and pass it through main logging macro. The function `init` just initializes logger configuration as described above.

```
int main(int, char**) {
    init();
    verbose_logger_t<level> log = repository_t::instance().root<level>();

    BH_LOG(log, level::debug, "log message using macro API");    
    return 0;
}
```

### For those who despise macros
Sometimes it is useful to handle log object directly without using any magic macro. Shortly I'll describe it now. For more advanced documentation you should go to the logger object's reference.

Every log object has `open_record` method which accepts attributes set and returns `log_record_t` object if that attributes set passes filtering stage. If not - an invalid `log_record_t` object is returned. For that reason returned record object must be checked for validity using `record.valid()` method. For our case log record will be created anyway, because we don't have any filters registered now.

The next thing we must do after checking record's validity - is to add message attribute into the record, because our formatter requires it, otherwise we got an formatting exception, which will be eaten by the logger internally or passed to the fallback logger. Entire function is:

```
template<typename Log>
void debug(Log& log, level lvl, const char* message) {    
    log::record_t record = log.open_record(lvl);
    if (record.valid()) {        
        record.attributes.insert({
            keyword::message() = message
        });
        log.push(std::move(record));
    }
}
```

And the usage is:

```
debug(log, level::debug, "log message using log object directly");
```

The complete example code:

```
#include <blackhole/blackhole.hpp>

//! This example demonstrates the a bit extended blackhole logging library usage and its features.

/*! - detailed formatters and sinks configuration;
 *  - logger usage without macro.
 */

using namespace blackhole;

// As always specify severity enumeration.
enum class level {
    debug
};

// Here we are going to configure our string/stream frontend and to register it.
void init() {
    // Configure string formatter.
    // Pattern syntax behaves like usual substitution for placeholder. For example if attribute
    // named `severity` has value `2`, then pattern [%(severity)s] will produce: [2].
    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s] <%(severity)s>:: %(message)s";

    // Configure stream sink to write into stdout (also stderr can be configured).
    sink_config_t sink("stream");
    sink["output"] = "stdout";

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "root", { frontend } };

    repository_t::instance().add_config(config);
}

// Here it's an example how to create and handle log events without using any macros at all.
template<typename Log>
void debug(Log& log, level lvl, const char* message) {
    // Tries to create log record. Returns invalid record object if created log record couldn't
    // pass filtering stage.
    // For our case it will be created anyway, because we don't have any filters registered now.
    log::record_t record = log.open_record(lvl);
    if (record.valid()) {
        // Manually insert message attribute into log record attributes set using keyword API.
        // Formatter will break up if some attributes it needed don't exist where formatting
        // occures.
        record.attributes.insert({
            keyword::message() = message
        });
        log.push(std::move(record));
    }
}

int main(int, char**) {
    init();
    verbose_logger_t<level> log = repository_t::instance().root<level>();

    BH_LOG(log, level::debug, "log message using macro API");
    debug(log, level::debug, "log message using log object directly");

    return 0;
}
```

After executing this program the next log messages will be printed on the console:

    [1396271998.662163] <0>:: log message using macro API
    [1396271998.662336] <0>:: log message using log object directly

As you can see, output format is completely under our control and can be easily changed.

Next tutorials shows you:

* How to inject intermediate attributes mapping to have, for example ISO-8601 formatted date-time instead of raw timestamp;
* How to apply log record filtering;
* How to transport additional attributes (both registered and inlined) into the log record and how to attach attribute to the logger object;
* How to build more complex loggers an example of logstash frontend.

[Back to contents](contents.md)